bool
VectorEngine::VectorMemPort::startTranslation(Addr addr, uint8_t *data,
    uint64_t size, BaseTLB::Mode mode, ThreadContext *tc, uint64_t req_id,
    uint8_t channel)
{
    Process *p = tc->getProcessPtr();
    Addr addra = p->pTable->pageAlign(addr);
    Addr addrb = p->pTable->pageAlign(addr + size - 1);
    assert(addra == addrb);

    uint8_t *ndata = new uint8_t[size];
    if(data != NULL) {
        assert(mode == BaseTLB::Write);
        memcpy(ndata, data, size);
    }

    MemCmd cmd = (mode == BaseTLB::Write) ? MemCmd::WriteReq :
    MemCmd::ReadReq;


    const Addr pc = tc->instAddr();
    RequestPtr req = std::make_shared<Request>(addr, size, 0, pc, tc->contextId());
    //owner->VectorCacheMasterId

    req->taskId = tc->getCPU->taskId();//what is the use

    VecMemTranslation* translaion = new VecMemTranslation(owner);
    tc->getTlbPtr()->translateTiming(req, tc, translation, mode);

    if(translaion->fault == NoFault) {
        PacketPtr pkt = new VectorPacket(req, cmd, req_id, channel);
        //reqid and channel is new
        pkt->dataDynamic(ndata);
        

    }
}