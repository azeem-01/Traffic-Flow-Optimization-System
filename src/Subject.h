#ifndef SUBJECT_H
#define SUBJECT_H
#include <list>
#include "Observer.h"
using namespace std;
class Subject 
{
    list<Observer*> ObserverList;  
public:
    virtual ~Subject() {}
    void addObserver(Observer* o)
    {
        ObserverList.push_back(o);      
    }
    void removeObserver(Observer* o)
    {
        ObserverList.remove(o);        
    }
    void notifyObservers()
    {
        for (Observer* o : ObserverList)
            o->update();   
    }
};
#endif