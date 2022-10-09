#ifndef __LEARNING_GEM5_HELLO_OBJECT_HH__
#define __LEARNING_GEM5_HELLO_OBJECT_HH__

#include <string>

#include "learning_gem5/part2/goodbye_object.hh"
#include "params/HelloObject.hh"
#include "sim/sim_object.hh"

namespace gem5
{

class HelloObject: public SimObject
{
  private:
    void processEvent();

    EventFunctionWrapper event;

    GoodbyeObject* goodbye;

    const std::string myName;

    const Tick latency;
    //interval between two events

    int timesLeft;
    //number of fires
  public:
    HelloObject(const HelloObjectParams &p);

    void startup();
    //We will initially schedule the event in the startup()
    //function we added to the HelloObject class. The startup()
    //function is where SimObjects are allowed to schedule internal events.
};

}

#endif