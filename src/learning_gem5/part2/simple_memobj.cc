#include "learning_gem5/part2/simple_memobj.hh"

#include "base/trace.hh"
#include "debug/SimpleMemobj.hh"

namespace gem5
{

SimpleMemobj::SimpleMemobj(const SimpleMemobjParams &params):
    SimObject(params),
    instPort(params.name + ".inst_port", this),
    dataPort(params.name + ".data_port", this),
    memPort(params.name + ".mem_side", this),
    blocked(false)
{

}

Port& SimpleMemobj::getPort(const std::string &if_name, PortID idx)
{
    panic_if(idx != InvalidPortID,
    "This object doesn't support vector ports\n");

    if (if_name == "mem_side") {
        return memPort;
    } else if (if_name == "data_port") {
        return dataPort;
    } else if (if_name == "inst_port") {
        return instPort;
    } else {
        //pass to super class
        return SimObject::getPort(if_name, idx);
    }
}

AddrRangeList SimpleMemobj::CPUSidePort::getAddrRanges() const
{
    return owner->getAddrRanges();
}

void SimpleMemobj::CPUSidePort::recvFunctional(PacketPtr pkt)
{
    return owner->handleFunctional(pkt);
    //calls next function:SimpleMemobj::handleFunctional
}

void SimpleMemobj::handleFunctional(PacketPtr pkt)
{
    memPort.sendFunctional(pkt);
    //?
}

AddrRangeList SimpleMemobj::getAddrRanges() const
{
    DPRINTF(SimpleMemobj, "Sending new requests\n");
    return memPort.getAddrRanges();
}

void SimpleMemobj::MemSidePort::recvRangeChange()
{
    owner->sendRangeChange();
}

void SimpleMemobj::sendRangeChange()
{
    instPort.sendRangeChange();
    dataPort.sendRangeChange();
}

bool SimpleMemobj::CPUSidePort::recvTimingReq(PacketPtr pkt)
{
    if (!owner->handleRequest(pkt)) {
        needRetry = true;
        return false;
    }
    return true;
}

bool SimpleMemobj::handleRequest(PacketPtr pkt)
{
    if (blocked) {
        return false;
    } else {
        DPRINTF(SimpleMemobj, "Got address for add %#x", pkt->getAddr());
        blocked = true;
        memPort.sendPacket(pkt);
        return true;
    }
}

void SimpleMemobj::MemSidePort::sendPacket(PacketPtr pkt)
{
    panic_if(blockedPacket != nullptr, "should never resend the packet\n");
    if (!sendTimingReq(pkt)) {
        blockedPacket = pkt;
        //many times of retry?
    }
}

void SimpleMemobj::MemSidePort::recvReqRetry()
{
    assert(blockedPacket != nullptr);
    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;
    sendPacket(pkt);
}

bool SimpleMemobj::MemSidePort::recvTimingResp(PacketPtr pkt)
{
    return owner->handleResponse(pkt);
    //?
}

bool SimpleMemobj::handleResponse(PacketPtr pkt)
{
    assert(blocked);
    DPRINTF(SimpleMemobj, "Got response for addr %#x\n", pkt->getAddr());

    blocked = false;

    //simply forward to the memory port
    if (pkt->req->isInstFetch()) {
        instPort.sendPacket(pkt);
    } else {
        dataPort.sendPacket(pkt);
    }

    instPort.trySendRetry();
    dataPort.trySendRetry();

    return true;
}

void SimpleMemobj::CPUSidePort::sendPacket(PacketPtr pkt)
{
    panic_if(blockedPacket != nullptr,
    "should never try to send if blocked!\n");

    if (!sendTimingResp(pkt)){
        blockedPacket = pkt;
    }
    //This function calls sendTimingResp which will in
    //turn call recvTimingResp on
    //the peer master port. If this call fails and the peer port is currently
    //blocked, then we store the packet to be sent later.
}

void SimpleMemobj::CPUSidePort::recvRespRetry()
{
    assert(blockedPacket != nullptr);

    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;

    sendPacket(pkt);
}

void SimpleMemobj::CPUSidePort::trySendRetry()
{
    if (needRetry && blockedPacket == nullptr) {
        needRetry = false;
        DPRINTF(SimpleMemobj, "Sending retry req for %d\n", id);
        sendRetryReq();
    }
}

}