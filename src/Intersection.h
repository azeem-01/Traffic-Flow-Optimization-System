#ifndef INTERSECTION_H
#define INTERSECTION_H
#include <string>
#include <list>
#include "Subject.h"
#include "Road.h"
#include "Observer.h"

using namespace std;

const int YELLOW_TICKS = 15; //1.5s delay

class Intersection : public Subject
{
    int id;
    string name;
    float Tg;      //green time
    float x, y;
    list<Road*> inRoads;
    Road* activeRoad;
    int signalTimer;
    bool inYellow;

public:
    Intersection(int ID, string Name, float GreenTime, float X, float Y)
    {
        id = ID;
        name = Name;
        Tg = GreenTime;
        x = X;
        y = Y;
        signalTimer = (int)Tg;
        inYellow = false;
        activeRoad = nullptr;
    }
    //getters
    int getId() { return id; }
    string getName() { return name; }
    float getTg() { return Tg; }
    float getX() { return x; }
    float getY() { return y; }
    list<Road*>& getInRoads() { return inRoads; }
    
    void setTg(float t) { Tg = t; }

    void addIncomingRoad(Road* r)
    {
        inRoads.push_back(r);
        if (inRoads.size() == 1)
        {
            r->setG(1);
            activeRoad = r;
        }
    }

    void tick()
    {
        if (inRoads.empty()) return;
        signalTimer--;
        if (signalTimer <= 0)
        {
            if (!inYellow)
            {
                //switch to yellow first
                inYellow = true;
                signalTimer = YELLOW_TICKS;
                if (activeRoad != nullptr)
                    activeRoad->setGTimer(YELLOW_TICKS); //road enters yellow
            }
            else
            {
                //yellow done, switch to next road
                inYellow = false;
                updateSignal();
                signalTimer = (int)Tg;
            }
        }
        notifyObservers();
    }
private:
    void updateSignal()
    {
        if (inRoads.empty()) return;
        for (Road* r : inRoads)
        {
            r->setG(0);
            r->setGTimer(0);
        }
        Road* longest = nullptr;
        int maxQ = 0; //start at 0 to cycle if all empty
        for (Road* r : inRoads)
        {
            if (r->getQueueSize() > maxQ)
            {
                maxQ = r->getQueueSize();
                longest = r;
            }
        }
        
        if (longest != nullptr)
        {
            activeRoad = longest;
        }
        else
        {
            //round robin cycle if no queues
            bool pickNext = false;
            Road* nextRoad = nullptr;
            for (Road* r : inRoads)
            {
                if (pickNext) {
                    nextRoad = r;
                    pickNext = false;
                    break;
                }
                if (r == activeRoad) pickNext = true;
            }
            if (pickNext || nextRoad == nullptr) 
                activeRoad = inRoads.front();
            else 
                activeRoad = nextRoad;
        }
        activeRoad->setG(1);
    }
};
#endif