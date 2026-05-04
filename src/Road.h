#ifndef ROAD_H
#define ROAD_H
#include <string>
#include <cmath>
#include "Subject.h"
#include "Vehicle.h"
#include "Queue.h"
using namespace std;

const float ALPHA = 0.15f;  //BPR congestion sensitivity
const float BETA = 4.0f;   //BPR nonlinearity factor
class Road : public Subject
{
    int fromNode;
    int toNode;
    float l;    //length
    float v;    //max speed
    int c;      //capacity
    int f;      //current vehicle count
    float w;    //travel time
    float wFree;//free flow travel time = l/v
    float rho;  //congestion level = f/c
    int g;      //signal: 1=green, 0=red
    int gTimer; //yellow phase countdown (>0 means yellow)
    float mu;   //max vehicles discharged per tick
    Queue<Vehicle*> Q; //vehicles waiting at downstream intersection

public:
    Road(int from, int to, float length, float maxSpeed, int cap, float discharge)
    {
        fromNode = from;
        toNode = to;
        l = length;
        v = maxSpeed;
        c = cap;
        mu = discharge;
        f = 0;
        g = 0;
        gTimer = 0;
        rho = 0.0f;
        wFree = l / v;
        w = wFree;
    }
    //getters
    int getFromNode() { return fromNode; }
    int getToNode() { return toNode; }
    float getW() { return w; }
    float getWFree() { return wFree; }
    float getRho() { return rho; }
    int getF() { return f; }
    int getC() { return c; }
    int getG() { return g; }
    int getGTimer() { return gTimer; }
    int getQueueSize() { return Q.getLength(); }
    //setters
    void setG(int signal) { g = signal; }
    void setGTimer(int t) { gTimer = t; }
    void setF(int flow) { f = flow; }
    void setL(float length) { l = length;    wFree = l / v; }
    void setV(float maxSpeed) { v = maxSpeed;  wFree = l / v; }
    void setC(int cap) { c = cap; }
    void setMu(float discharge) { mu = discharge; }
    void incrementFlow() { f++; }
    list<Vehicle*> getQueueList()
    {
        list<Vehicle*> result;
        QNode<Vehicle*>* curr = Q.getFront();
        while (curr != nullptr)
        {
            result.push_back(curr->d);
            curr = curr->next;
        }
        return result;
    }
    void vehicleFinished(Vehicle* veh)
    {
        f--;
        if (f < 0) f = 0;
        Q.enqueue(veh);
        veh->status = "waiting";
        veh->currEdge = this;
        veh->nextEdge = nullptr;
    }

    void addVehicle(Vehicle* veh)
    {
        if (f >= c)
        {
            Q.enqueue(veh);
            veh->status = "waiting";
            veh->currEdge = this;
            veh->nextEdge = nullptr;
            return;
        }
        f++;
        veh->currEdge = this;
        veh->nextEdge = nullptr;
        veh->rv = w;      //remaining time = current travel time
        veh->status = "moving";
    }
    void tick()
    {
        //yellow phase, no discharge during yellow
        if (gTimer > 0)
        {
            gTimer--;
            updateRho();
            updateW();
            notifyObservers();
            return;
        }
        //discharge queue if green and queue not empty
        if (g == 1 && !Q.isEmpty())
        {
            int canDischarge = (int)mu;
            while (canDischarge > 0 && !Q.isEmpty())
            {
                Vehicle* veh = Q.getFrontElement();
                Road* next = veh->nextEdge;
                //check next road capacity before discharging
                if (next != nullptr && next->getF() < next->getC())
                {
                    Q.dequeue();
                    next->addVehicle(veh);
                    canDischarge--;
                }
                else
                    break;
            }
        }
        updateRho();
        updateW();
        notifyObservers();
    }
private:
    void updateRho()
    {
        rho = (c > 0) ? (float)f / c : 1.0f;
    }
    void updateW()
    {
        w = wFree * (1.0f + ALPHA * pow(rho, BETA));
    }
};
#endif