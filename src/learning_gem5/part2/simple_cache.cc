#include "learning_gem5/part2/simple_cache.hh"

#include "base/compiler.hh"
#include "base/random.hh"
#include "debug/SimpleCache.hh"
#include "sim/system.hh"

namespace gem5
{
SimpleCache::SimpleCache (const SimpleCacheParams& params):
    ClockedObject(params),
    latency(params.latency),
    blockSize(params.system->cacheLineSize()),
    capacity(params.size / blockSize),
    memPort(params.name + ".mem_side", this),
    blocked(false), outstandingPacket(nullptr), waitingPortId(-1),
    stats(this)
{
    for (int i = 0; i < params.port_cpu_side_connection_count; ++i) {
        cpuPorts.emplace_back(name() + csprintf(".cpu_side[%d]", i), i, this);
    }
}

Port &
SimpleCache::getPort(const std::string &if_name, PortID idx)
{
    if (if_name == "mem_side") {
        panic_if(idx != InvalidPortID,
                 "Mem Side of simpcache isn't a Vec Port");
        return memPort;
    } else if (if_name == "cpu_side" && idx < cpuPorts.size()) {
        //should create these in constructor
        return cpuPorts[idx];
    } else {
        //pass along to super class
        return ClockedObject::getPort(if_name, idx);
    }
}

AddrRangeList SimpleCache::getAddrRanges() const
{
    DPRINTF(SimpleCache, "Sending new ranges\n");
    //Just use the same ranges as whatever is on the memory side.
    return memPort.getAddrRanges();
}

AddrRangeList
SimpleCache::CPUSidePort::getAddrRanges() const
{
    return owner->getAddrRanges();
}


void SimpleCache::CPUSidePort::trySendRetry()
{
    //called by RecvRespRetry
    if (needRetry && blockedPacket == nullptr) {
        DPRINTF(SimpleCache, "Sending retry req.\n");
        sendRetryReq();
        //Send a retry to the request port that previously attempted
        //a sendTimingReq to this response port and failed.
    }
}

void SimpleCache::handleFunctional(PacketPtr pkt)
{
    if (accessFunctional(pkt)) { //see if cache hits
        pkt->makeResponse();
    } else {
        memPort.sendFunctional(pkt);
    }
}

void SimpleCache::CPUSidePort::recvFunctional(PacketPtr pkt)
{
    //just forward to the cache.
    return owner->handleFunctional(pkt);
}

bool SimpleCache::CPUSidePort::recvTimingReq(PacketPtr pkt)
{
    DPRINTF(SimpleCache, "Got request %s\n", pkt->print());
    if (blockedPacket || needRetry) {
        DPRINTF(SimpleCache, "Request blocked\n");
        needRetry = true;
        return false;
    }

    if (!owner->handleRequest(pkt, id)) {
        DPRINTF(SimpleCache, "Request failed\n");
        needRetry = true;
        return false;
    } else {
        DPRINTF(SimpleCache, "Request succeeded\n");
        return true;
    }
}

void SimpleCache::CPUSidePort::recvRespRetry()
{
    assert(blockedPacket != nullptr);
    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;
    //before resend, set the blockedpacket to null

    DPRINTF(SimpleCache, "Retrying response pkt %s\n", pkt->print());
    sendPacket(pkt);

    trySendRetry();
}

void SimpleCache::CPUSidePort::sendPacket(PacketPtr pkt)
{
    panic_if(blockedPacket != nullptr,
    "should never try to send if blocked!\n");
    DPRINTF(SimpleCache, "sending %s to cpu\n",pkt->print());
    if (!sendTimingResp(pkt)) {
        DPRINTF(SimpleCache, "failed!\n");
        blockedPacket = pkt;
    }
    //This function calls sendTimingResp which
    //will in turn call recvTimingResp on
    //the peer master port. If this call
    //fails and the peer port is currently
    //blocked, then we store the packet to be sent later.
}

bool SimpleCache::handleRequest(PacketPtr pkt, int port_id)
{
    if (blocked) {
        return false;
    }
    DPRINTF(SimpleCache, "Got request for addr %#x\n", pkt->getAddr());

    blocked = true;
    waitingPortId = port_id;

    schedule(new EventFunctionWrapper([this,pkt]{accessTiming(pkt); },
    name() + ".accessEvent", true), clockEdge(latency));
    //?
    return true;
}

void SimpleCache::accessTiming(PacketPtr pkt)
{
    bool hit = accessFunctional(pkt);
    if (hit) {
        stats.hits++;
        pkt->makeResponse();
        //This converts the packet from a
        //request packet to a response packet.
        sendResponse(pkt);
        //Then, we can send the response back to the CPU.
    } else {
        stats.misses++;
        missTime = curTick();
        Addr addr = pkt->getAddr();
        Addr block_addr = pkt->getBlockAddr(blockSize);
        unsigned size = pkt->getSize();
        if (addr == block_addr && size == blockSize) {
            DPRINTF(SimpleCache, "forwarding packet\n");
            memPort.sendPacket(pkt);
        } else {
            DPRINTF(SimpleCache, "Upgrading packet to block size\n");
            panic_if(addr - block_addr + size > blockSize, "Linespan\n");
            MemCmd cmd;
            if (pkt->isWrite() || pkt->isRead()) {
                cmd = MemCmd::ReadReq;
                //read from memory
            } else {
                panic("Unknown request\n");
            }
            PacketPtr new_pkt = new Packet(pkt->req, cmd, blockSize);
            //src/mem/packet.cc
            new_pkt->allocate();

            outstandingPacket = pkt;
            memPort.sendPacket(new_pkt);
        }
    }
}

void SimpleCache::insert(PacketPtr pkt)
{
    if (cacheStore.size() >= capacity) {
        int bucket, bucket_size;
        do {
            bucket = random_mt.random(0, (int)cacheStore.bucket_count() - 1);
        } while ( (bucket_size = cacheStore.bucket_size(bucket)) == 0 );
        //find a non-empty bucket to evict
        auto block = std::next(cacheStore.begin(bucket),
                                random_mt.random(0, bucket_size - 1));
        RequestPtr req =
        std::make_shared<Request>(block->first, blockSize, 0, 0);
        //Addr paddr, unsigned size, Flags flags, RequestorID id)
        PacketPtr new_pkt = new Packet(req, MemCmd::WritebackDirty, blockSize);
        //no matter dirty or not, assume dirty
        new_pkt->dataDynamic(block->second);
        DPRINTF(SimpleCache, "Writing cache back %s\n", pkt->print());
        memPort.sendTimingReq(new_pkt);
        cacheStore.erase(block->first);//key
    }
    uint8_t* data = new uint8_t[blockSize];
    cacheStore[pkt->getAddr()] = data;
    pkt->writeDataToBlock(data,blockSize);
}

void SimpleCache::sendResponse(PacketPtr pkt)
//called by handleresponse
{
    int port = waitingPortId;
    DPRINTF(SimpleCache, "Sending resp for addr %#x\n", pkt->getAddr());

    blocked = false;
    waitingPortId = -1;

    cpuPorts[port].sendPacket(pkt);
    for (auto& port : cpuPorts) {
        port.trySendRetry();
    }
}

void SimpleCache::sendRangeChange() const
{
    for (auto& port : cpuPorts) {
        port.sendRangeChange();
    }
}

bool SimpleCache::handleResponse(PacketPtr pkt)
{
    assert(blocked);
    DPRINTF(SimpleCache, "Got response for addr %#x", pkt->getAddr());
    insert(pkt);//1
    stats.missLatency.sample(curTick() - missTime);
    if (outstandingPacket != nullptr) {
        //miss(receive from axi)
        accessFunctional(outstandingPacket);
        outstandingPacket->makeResponse();
        //Take a request packet and modify it in place to be suitable for
        //returning as a response to that request.
        delete pkt;
        pkt = outstandingPacket;
        outstandingPacket = nullptr;
    } //else, the packet contains the data it needs
    sendResponse(pkt);
    return true;
}

bool SimpleCache::accessFunctional(PacketPtr pkt)
{
    Addr block_addr = pkt->getBlockAddr(blockSize);
    auto it = cacheStore.find(block_addr);
    if (it != cacheStore.end()) {//is found
        if (pkt->isWrite()) {
            pkt->writeDataToBlock(it->second, blockSize);
        } else if (pkt->isRead()) {
            pkt->setDataFromBlock(it->second, blockSize);
        } else {
            panic("Unknown type\n");
        }
        return true;
    }
    return false;
}

void SimpleCache::MemSidePort::sendPacket(PacketPtr pkt)
{
    panic_if(blockedPacket != nullptr, "should never resend the packet\n");
    if (!sendTimingReq(pkt)) {
        blockedPacket = pkt;
    }
}

bool SimpleCache::MemSidePort::recvTimingResp(PacketPtr pkt)
{
    return owner->handleResponse(pkt);
}

void SimpleCache::MemSidePort::recvReqRetry()
{
    assert(blockedPacket != nullptr);
    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;

    sendPacket(pkt);
}

void SimpleCache::MemSidePort::recvRangeChange()
{
    owner->sendRangeChange();
}

SimpleCache::SimpleCacheStats::SimpleCacheStats(statistics::Group *parent)
      : statistics::Group(parent),
      ADD_STAT(hits, statistics::units::Count::get(), "Number of hits"),
      ADD_STAT(misses, statistics::units::Count::get(), "Number of misses"),
      ADD_STAT(missLatency, statistics::units::Tick::get(),
               "Ticks for misses to the cache"),
      ADD_STAT(hitRatio, statistics::units::Ratio::get(),
               "Ratio of hits to the total access to cache",
               hits / (hits + misses))
{
    missLatency.init(16); // number of buckets
}


}