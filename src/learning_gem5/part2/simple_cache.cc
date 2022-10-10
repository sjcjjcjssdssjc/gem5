SimpleCache::SimpleCache (SimpleCacheParams& params):
    MemObject(params),
    latancy(params->latancy),
    blockSize(params->system->cacheLineSize),
    capacity(params->size / blockSize),
    memPort(params->name + ".mem_side", this),
    blocked(false), outstandingPacket(nullptr), waitingPortId(-1)
    {
        for (int i = 0; i < params->port_cpu_side_connection_count; ++i) {
            cpuPorts.emplace_back
            (name() + csprintf(".cpu_side[%d]", i), i, this);
        }
    }

    BaseSlavePort&
    SimpleCache::getSlavePort(const std::string& if_name, PortId idx)
    {
        if (if_name == "cpu_side" && idx < cpuPorts.size()) {
            return cpuPorts[idx];
        } else {
            return MemObject::getSlavePort(if_name, idx);
        }
    }

    bool SimpleCache::handleRequest(PacketPtr pkt, int pord_id)
    {
        if (blocked) {
            return flase;
        }
        DPRINTF(SimpleCache, "Got request for addr %#x\n", pkt->getAddr());

        blocked = true;
        waitingPortId = port_id;

        schedule(new EventFunctionWrapper([this,pkt]{accessTiming(pkt); },
        name() + ".accessEvent", true), clockEdge(latancy));
        //?
    }

    void SimpleCache::accessTiming(PacketPtr pkt)
    {
        bool hit = accessFunctional(pkt);
        if (hit) {
            pkt->makeResponse();
            //This converts the packet from a
            //request packet to a response packet.
            sendResponse(pkt);
            //Then, we can send the response back to the CPU.
        } else {
            Addr = pkt->getAddr();
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
                    cmd = MemCmd::
                } else {
                    panic("Unknown request\n");
                }
            }
            PacketPtr new_pkt = new Packet(pkt->req, cmd, blockSize);
            /*
                Packet(const RequestPtr &_req, MemCmd _cmd)
                : cmd(_cmd), id((PacketId)_req.get()), req(_req),
                data(nullptr), addr(0), _isSecure(false), size(0Unsigned),
                _qosValue(0),
            */
            new_pkt->allocate();

            outstandingPacket = pkt;
            memPort.sendPacket(new_pkt);
        }
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

    void SimpleCache::sendResponse(PacketPtr pkt)
    {
        int port = waitingPortId;

        blocked = false;
        waitingPortId = -1;

        cpuPorts[port].sendPacket(pkt);
        for (auto& port : cpuPorts) {
            port.trySendRetry();
        }
    }

    bool SimpleCache::handleResponse(PacketPtr pkt)
    {
        assert(blocked);
        DPRINTF(SimpleCache, "Got response for addr %#x", pkt->getAddr());
        insert(pkt);//1
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
        sendResponse(packet);
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

    void SimpleMemobj::MemSidePort::sendPacket(PacketPtr pkt)
    {
        panic_if(blockedPacket != nullptr, "should never resend the packet\n");
        if (!sendTimingReq(pkt)) {
            blockedPacket = pkt;
        }
    }