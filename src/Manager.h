#ifndef MANAGER_H
#define MANAGER_H
#include <list>
#include <cmath>
#include <cstdlib>
#include "Observer.h"
#include "Graph.h"
#include "Road.h"
#include "Intersection.h"
#include "Vehicle.h"
#include "Initializer.h"
using namespace std;

const float OBJ_A = 0.5f;  //queue weight in objective function
const float OBJ_B = 0.5f;  //congestion weight in objective function
const int   DELAY = 30;    //auto add interval in ticks (3s at 100ms)
const float DISAPPEAR = 50.0f; //when vehicle is removed from GUI

class Manager : public Observer
{
    AdjList<int>* graph;
    list<Intersection*> intersections;
    list<Road*> roads;
    list<Vehicle*> vehicles;
    int vehCount;     //vehicle id counter
    int tickCount;
    float avgW;         //average travel time
    float totalDelay;
    float avgRho;       //average congestion
    int throughput;   //vehicles arrived
    float objValue;     //objective function value

public:
    Manager(Initializer& init)
    {
        graph = init.getGraph();
        intersections = init.getIntersections();
        roads = init.getRoads();
        vehCount = 0;
        tickCount = 0;
        avgW = 0;
        totalDelay = 0;
        avgRho = 0;
        throughput = 0;
        objValue = 0;
        for (Road* r : roads)
            r->addObserver(this);
        //preload incoming roads for each intersection
        for (Intersection* inter : intersections)
        {
            for (Road* r : roads)
            {
                if (r->getToNode() == inter->getId())
                    inter->addIncomingRoad(r);
            }
        }
    }
    //getters
    float getAvgW() { return avgW; }
    float getTotalDelay() { return totalDelay; }
    float getAvgRho() { return avgRho; }
    int getThroughput() { return throughput; }
    float getObjValue() { return objValue; }
    int getTickCount() { return tickCount; }
    list<Vehicle*>& getVehicles() { return vehicles; }
    list<Road*>& getRoads() { return roads; }
    list<Intersection*>& getIntersections() { return intersections; }

    Road* findRoad(int from, int to)
    {
        for (Road* r : roads)
        {
            if (r->getFromNode() == from && r->getToNode() == to)
                return r;
        }
        return nullptr;
    }
    void addVehicle(int src, int dst)
    {
        list<int> route = graph->shortestPath(src, dst);
        if (route.size() < 2)
            return;
        auto it = route.begin();
        int fromNode = *it++;
        int toNode = *it;
        Road* first = findRoad(fromNode, toNode);
        if (first == nullptr)
            return;
        Vehicle* veh = new Vehicle();
        veh->id = vehCount++;
        veh->src = src;
        veh->dst = dst;
        veh->route = route;
        veh->currEdge = nullptr;
        veh->nextEdge = nullptr;
        veh->disappearTimer = DISAPPEAR;
        first->addVehicle(veh);
        vehicles.push_back(veh);
    }
    void update() override  //called by road on congestion change
    {
        syncWeights();
        rerouteVehicles();
    }
    void tick()
    {
        tickCount++;
        syncWeights();  //step 1: fresh edge weights
        updateVehicles();   //step 2: rv decrement + transitions
        autoAdd();      //step 3: spawn vehicles
        for (Intersection* i : intersections) i->tick();//step 4: signal update
        for (Road* r : roads)   //step 5: road update + notify  
            r->tick();
        updateMetrics();    //step 6
        calcObjective();    //step 7
        cleanArrived(); //step 8: remove expired vehicles
    }
    void syncWeights()
    {
        for (Road* r : roads)
            graph->updateEdgeWeight(r->getFromNode(), r->getToNode(), r->getW());
    }
    void updateVehicles()
    {
        for (Vehicle* veh : vehicles)
        {
            if (veh->status == "arrived")
            {
                veh->disappearTimer--;  //countdown for GUI removal
                continue;
            }
            if (veh->status != "moving")
                continue;
            veh->rv -= 0.1f;           //rv(t+1) = rv(t) - 0.1(tick duration)
            if (veh->rv <= 0.0f)
            {
                Road* curr = veh->currEdge;
                bool lastRoad = (veh->route.size() <= 2);
                if (!veh->route.empty())
                    veh->route.pop_front(); //remove current node
                if (!lastRoad && curr != nullptr)
                {
                    curr->vehicleFinished(veh);
                    if (veh->route.size() >= 2)
                    {
                        auto it = veh->route.begin();
                        int fromNode = *it++;
                        int toNode = *it;
                        veh->nextEdge = findRoad(fromNode, toNode); //store next road for discharge
                    }
                }
                else
                {
                    if (curr != nullptr)
                    {
                        int newF = curr->getF() - 1;
                        if (newF < 0) newF = 0;
                        curr->setF(newF);
                    }
                    veh->status = "arrived";
                    veh->disappearTimer = DISAPPEAR;
                    veh->currEdge = nullptr;
                    veh->nextEdge = nullptr;
                }
            }
        }
    }
    void rerouteVehicles()
    {
        for (Vehicle* veh : vehicles)
        {
            if (veh->status != "moving" || veh->currEdge == nullptr)
                continue;
            
            int fromNode = veh->currEdge->getFromNode();
            int toNode = veh->currEdge->getToNode();
            
            list<int> newRoute = graph->shortestPath(toNode, veh->dst);
            if (!newRoute.empty())
            {
                newRoute.push_front(fromNode); // Keep current road start to match pop_front logic
                veh->route = newRoute;
            }
        }
    }
    void autoAdd()
    {
        if (tickCount % DELAY != 0)
            return;
        if (intersections.size() < 2)
            return;
        int total = (int)intersections.size();
        int srcIdx = rand() % total;
        int dstIdx = rand() % total;
        while (dstIdx == srcIdx)
            dstIdx = rand() % total;
        auto it = intersections.begin();
        advance(it, srcIdx);
        int src = (*it)->getId();
        it = intersections.begin();
        advance(it, dstIdx);
        int dst = (*it)->getId();
        addVehicle(src, dst);
    }
    void updateMetrics()
    {
        int   active = 0;
        float sumW = 0;
        float sumDly = 0;
        throughput = 0;
        for (Vehicle* veh : vehicles)
        {
            if (veh->status == "arrived")
            {
                throughput++;
                continue;
            }
            active++;
            if (veh->currEdge != nullptr)
            {
                sumW += veh->currEdge->getW();
                sumDly += veh->currEdge->getW() - veh->currEdge->getWFree();
            }
        }
        avgW = (active > 0) ? sumW / active : 0.0f;
        totalDelay = sumDly;
        float sumRho = 0;
        for (Road* r : roads)
            sumRho += r->getRho();
        avgRho = (!roads.empty()) ? sumRho / roads.size() : 0.0f;
    }
    void calcObjective()
    {
        objValue = 0;
        for (Road* r : roads)
            objValue += OBJ_A * r->getQueueSize() + OBJ_B * pow(r->getRho(), 2.0f);
    }
    void cleanArrived()
    {
        vehicles.remove_if([](Vehicle* v)
            {
                if (v->status == "arrived" && v->disappearTimer <= 0)
                {
                    delete v;
                    return true;
                }
                return false;
            });
    }
    ~Manager()
    {
        for (Vehicle* v : vehicles)
            delete v;
    }
};
#endif