#ifndef __LEARNING_GEM5_HELLO_OBJECT_HH__
#define __LEARNING_GEM5_HELLO_OBJECT_HH__

#include "params/HelloObject.hh"
#include "sim/sim_object.hh"

namespace gem5
{
class HelloObject: public SimObject
{
  private:
    void processevent();

    EventFunctionWrapper event;
  public:
    HelloObject(const HelloObjectParams &p);

    void startup();//?
};

}

#endif