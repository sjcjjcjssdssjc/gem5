#include "learning_gem5/part2/hello_object.hh"

#include "base/trace.hh"
#include "debug/Hello.hh"
#include "debug/HelloExample.hh"

namespace gem5
{

HelloObject::HelloObject(const HelloObjectParams &p):
    SimObject(p),
    // This is a C++ lambda. When the event is triggered, it will call the
    // processEvent() function. (this must be captured)
    event([this]{processEvent();}, name()),
    myName(p.name),
    goodbye(p.goodbye_object),
    latency(p.time_to_wait),
    timesLeft(p.number_of_fires)
    {
        DPRINTF(HelloExample, "Created the Hello Object!\n");
        panic_if(!goodbye, "need to have a non-null goodbye\n");
    }
    void HelloObject::startup()
    {
        schedule(event, latency);
    }
    void HelloObject::processEvent()
    {
        timesLeft--;
        DPRINTF(HelloExample, "Hello left time %d!\n",timesLeft);
        if (timesLeft == 0) {
            DPRINTF(HelloExample, "End of the events\n");
            goodbye->sayGoodbye(myName);
        } else {
            schedule(event, latency + curTick());
            //curTick is base function, representing the clock ticks elapsed
        }
    }

}