#ifndef __LEARNING_GEM5_PART2_SIMPLE_MEMOBJ_HH__
#define __LEARNING_GEM5_PART2_SIMPLE_MEMOBJ_HH__

#include "base/statistics.hh"
#include "mem/port.hh"
#include "params/SimpleMemobj.hh"
#include "sim/sim_object.hh"

namespace gem5
{

class SimpleMemobj:public SimObject
{
  private:
    class MemSidePort:public RequestPort
    {
      private:
        SimpleMemobj* owner;
        PacketPtr   blockedPacket;
        //why here(send blocked)
      public:
        MemSidePort(const std::string& name, SimpleMemobj* owner):
            RequestPort(name, owner), owner(owner),
            blockedPacket(nullptr)//why two owners
        {}
        void sendPacket(PacketPtr pkt);
        //send packet accross the port(send to)
      protected:
        bool recvTimingResp(PacketPtr ptr) override;
        void recvReqRetry() override;
        //firstly the SLAVE sendreqretry. So no blocked packet.
        void recvRangeChange() override;
    };

    class CPUSidePort:public ResponsePort
    {
      private:
        SimpleMemobj* owner;
        bool needRetry;
        PacketPtr   blockedPacket;
        //why here(resp blocked)
      public:
        CPUSidePort(const std::string& name, SimpleMemobj* owner):
            ResponsePort(name, owner), owner(owner), needRetry(false),
            blockedPacket(nullptr)//why two owners
        {}
        AddrRangeList getAddrRanges()const override;
        void sendPacket(PacketPtr pkt);
        //send packet accross the port(send to)
        void trySendRetry();
      protected:
        Tick recvAtomic(PacketPtr pkt) override
        { panic("recvAtomic unimpl."); }
        void recvFunctional(PacketPtr pkt) override;
        bool recvTimingReq(PacketPtr pkt) override;
        void recvRespRetry() override;

    };
    CPUSidePort instPort;
    CPUSidePort dataPort;

    MemSidePort memPort;
    bool blocked;
    //memory is blocked waiting for a response
    void handleFunctional(PacketPtr pkt);
    void sendRangeChange();
    bool handleResponse(PacketPtr pkt);
    bool handleRequest (PacketPtr pkt);
    AddrRangeList getAddrRanges() const;

  public:
    SimpleMemobj(const SimpleMemobjParams &params);
    Port& getPort(const std::string &if_name,
                  PortID idx = InvalidPortID) override;

};

}

#endif