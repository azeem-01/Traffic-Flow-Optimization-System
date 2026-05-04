#ifndef VEHICLE_H
#define VEHICLE_H
#include <string>
#include <list>
using namespace std;

class Road;
struct Vehicle
{
    int id;
    int src;
    int dst;
    Road* currEdge;     //current road
    Road* nextEdge;     //next road
    float rv;           //remaining travel time
    float disappearTimer; //countdown after arrival
    string status;      //moving, waiting, arrived
    list<int> route;    //remaining node IDs to visit
};
#endif