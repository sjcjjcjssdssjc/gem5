#include "cpu/vector/vector_lane.hh"

VectorLane::VectorLane()
{

}

VectorLane::~VectorLane()
{

}

void
VectorLane::issue(VectorEngine& vector_engine,
    RiscvISA::VectorStaticInst& insn,
    VectorDynInst* dyn_insn, ExecContextPtr& xc, uint64_t src1,
    uint64_t vtype, uint64_t vl,
    std::function<void(Fault fault)> done_callback)
{

}