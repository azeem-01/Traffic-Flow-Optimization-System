#ifndef INITIALIZER_H
#define INITIALIZER_H
#include <string>
#include "Graph.h"
#include "Road.h"
#include "Intersection.h"
using namespace std;

class Initializer
{
    AdjList<int>* graph;
    list<Intersection*> intersections;
    list<Road*> roads;


public:
    Initializer()
    {
        graph = new AdjList<int>(true); //directed graph

        buildCity();
    }
    //getters

    AdjList<int>* getGraph() { return graph; }
    list<Intersection*>& getIntersections() { return intersections; }
    list<Road*>& getRoads() { return roads; }

    void addIntersection(int id, string name, float Tg, float x, float y)
    {
        Intersection* i = new Intersection(id, name, Tg, x, y);
        intersections.push_back(i);
        graph->insertVertix(id);
    }

    void addRoad(int from, int to, float length, float maxSpeed, int cap, float discharge)
    {
        Road* r = new Road(from, to, length, maxSpeed, cap, discharge);
        roads.push_back(r);
        graph->insertEdge(from, to, r->getWFree());
    }
    void addBidirectionalRoad(int a, int b, float length, float maxSpeed, int cap, float discharge)
    {
        addRoad(a, b, length, maxSpeed, cap, discharge); //A->B
        addRoad(b, a, length, maxSpeed, cap, discharge); //B->A
    }
    void editIntersection(int id, float newTg)
    {
        for (Intersection* i : intersections)
            if (i->getId() == id) { i->setTg(newTg); return; }
    }
    void editRoad(int from, int to, float newL, float newV, int newC, float newMu)
    {
        for (Road* r : roads)
        {
            if (r->getFromNode() == from && r->getToNode() == to)
            {
                r->setL(newL);
                r->setV(newV);
                r->setC(newC);
                r->setMu(newMu);
                graph->updateEdgeWeight(from, to, r->getWFree());
                return;
            }
        }
    }
    void buildCity()
    {
        const int grid = 6;
        const float panelW = 1400.0f;
        const float panelH = 1200.0f;
        const float margin = 80.0f;
        float spacingX = (panelW - 2 * margin) / (grid - 1);
        float spacingY = (panelH - 2 * margin) / (grid - 1);
        //add intersections named R(row)C(col)
        for (int row = 0; row < grid; row++)
            for (int col = 0; col < grid; col++)
            {
                int id = row * grid + col + 1;
                string name = "R" + to_string(row + 1) + "C" + to_string(col + 1);
                float x = margin + col * spacingX;
                float y = margin + row * spacingY;
                addIntersection(id, name, 5.0f, x, y);
            }
        //horizontal bidirectional roads
        for (int row = 0; row < grid; row++)
            for (int col = 0; col < grid - 1; col++)
            {
                int a = row * grid + col + 1;
                int b = row * grid + col + 2;
                addBidirectionalRoad(a, b, spacingX, 50.0f, 15, 4.0f);
            }
        //vertical bidirectional roads
        for (int row = 0; row < grid - 1; row++)
            for (int col = 0; col < grid; col++)
            {
                int a = row * grid + col + 1;
                int b = (row + 1) * grid + col + 1;
                addBidirectionalRoad(a, b, spacingY, 50.0f, 15, 4.0f);
            }
    }
    ~Initializer()
    {
        delete graph;
        for (Intersection* i : intersections)
            delete i;
        for (Road* r : roads)
            delete r;
    }
};
#endif