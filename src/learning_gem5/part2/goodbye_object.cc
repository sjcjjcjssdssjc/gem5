#include "learning_gem5/part2/goodbye_object.hh"

#include "base/trace.hh"
#include "debug/HelloExample.hh"
#include "sim/sim_exit.hh"

namespace gem5
{

GoodbyeObject::GoodbyeObject(const GoodbyeObjectParams &params):
    SimObject(params),
    event([this]{processEvent();}, name()),
    bandwidth(params.write_bandwidth),
    bufferSize(params.buffer_size),
    bufferUsed(0),
    buffer(nullptr)
    {
        buffer = new char[bufferSize];
        DPRINTF(HelloExample, "Created the Goodbye Object!\n");
    }
    GoodbyeObject::~GoodbyeObject()
    {
        delete[] buffer;
    }
    void GoodbyeObject::processEvent()
    {
        DPRINTF(HelloExample, "Precessing the event\n");
        fillBuffer();
    }
    void GoodbyeObject::sayGoodbye(std::string othername)
    {
        DPRINTF(HelloExample, "Saying Goodbye to %s", othername);
        message = "GoodBye" + othername + "!! ";
        fillBuffer();
    }
    void GoodbyeObject::fillBuffer()
    {
        assert(message.length() > 0);
        int bytes_copied = 0;
        for (auto it = message.begin();
            it < message.end() && bufferUsed < bufferSize - 1;
            it++, bytes_copied++, bufferUsed++) {
            //copy the char into the buffer
            buffer[bufferUsed] = *it;
        }
        if (bufferUsed < bufferSize - 1) {
            DPRINTF(HelloExample, "Scheduling another fillbuffer in %d ticks",
                    bandwidth * bytes_copied);
            schedule(event, curTick() + bandwidth * bytes_copied);
            //To model the limited bandwidth, each time we write the
            //message to the buffer, we pause for the latency it takes
            //to write the message. We use a simple event to model this pause.
        } else {
            DPRINTF(HelloExample, "Goodbye world");
            //Be sure to take into account the time for the last bytes
            exitSimLoop(message, 0, curTick() + bandwidth * bytes_copied);
            //exit the simulation. This function takes three parameters,
            //the first is the message to return to the Python config script
            //(exit_event.getCause()), the second is the exit code,
            //and the third is when to exit.
        }
    }

}