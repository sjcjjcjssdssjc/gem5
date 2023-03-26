VectorEngine::VectorMemPort::VectorMemPort(const std::string& name, VectorEngine& owner,
            uint8_t channels) :
            MasterPort(name, owner), owner(owner)
{
    for(uint8_t i = 0; i < channels; i++) {
        lsq.push_back(std::deque<PacketPtr>());
    }
}

bool VectorEngine::VectorMemPort::startTranslation(Addr addr, uint8_t *data,
    uint64_t size, BaseTLB::Mode mode, ThreadContext *tc, uint64_t req_id,
    uint8_t channel)//sendTimingReadorWriteReq
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

        if(!sendTimingReq(pkt)) {
            lsq[channel].push_back(pkt);
        }

        delete translaion;
        return true;
    } else {
        translaion->fault->invoke(tc, NULL);
        delete translaion;
        return false;
    }
}

void
VectorEngine::issue(RiscvISA::VectorStaticInst& insn,
    VectorDynInst *dyn_insn,
    ExecContextPtr& xc ,uint64_t src1 ,uint64_t src2,uint64_t vtype,
    uint64_t vl, std::function<void(Fault fault)> done_callback)
{

}