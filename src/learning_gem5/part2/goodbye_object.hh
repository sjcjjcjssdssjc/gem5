#ifndef __LEARNING_GEM5_GOODBYE_OBJECT_HH__
#define __LEARNING_GEM5_GOODBYE_OBJECT_HH__

#include <string>

#include "params/GoodbyeObject.hh"
#include "sim/sim_object.hh"

namespace gem5
{

class GoodbyeObject:public SimObject
{
  private:
    void processEvent();

    void fillBuffer();
    EventFunctionWrapper event;

    float bandwidth;
    //the ticks per byte

    int bufferSize;
    int bufferUsed;
    //amount of the buffer that we have used so far

    char* buffer;

    std::string message;
    //the message that is put to the buffer
  public:
    GoodbyeObject(const GoodbyeObjectParams &p);
    ~GoodbyeObject();

    void sayGoodbye(std::string othername);
    //called by a outside object.
    //Fill the obj with goodbye message
};

}
#endif