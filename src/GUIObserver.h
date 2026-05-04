#ifndef GUI_H
#define GUI_H
#include "Observer.h"
#include <vcclr.h>

namespace SemesterProject
{
    ref class trafficSimulation;
}
class GUIObserver : public Observer
{
    gcroot<SemesterProject::trafficSimulation^> form;
public:
    GUIObserver(SemesterProject::trafficSimulation^ f)
    {
        form = f;
    }
    void update() override; 
};
#endif