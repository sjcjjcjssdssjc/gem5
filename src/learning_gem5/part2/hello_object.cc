#include "base/trace.hh"
#include "debug/HelloExample.hh"
//this is automatically generated
#include "learning_gem5/part2/hello_object.hh"

namespace gem5
{

HelloObject::HelloObject(const HelloObjectParams &p):
    SimObject(p), event([this]{processevent();}, name())
    {
        DPRINTF(HelloExample, "Created the Hello Object!\n");
    }
    void HelloObject::processevent()
    {

    }

}