#include "sim/ticked_object.hh"

class VectorLane;

class DataPath:public TickedObject
{
    class TimedFunc
    {
      public:
        TimedFunc(uint64_t cyclesLeft, std::function<void(void)> execute) :
            cyclesLeft(cyclesLeft), execute(execute) {}
        ~TimedFunc() {}

        uint64_t cyclesLeft;
        std::function<void(void)> execute;
    };
}