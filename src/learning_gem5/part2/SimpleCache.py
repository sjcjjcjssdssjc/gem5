from m5.params import *
from m5.proxy  import *
from m5.objects.ClockedObject import ClockedObject


class SimpleCache(ClockedObject):
    type = "SimpleCache"
    cxx_header = "learning_gem5/part2/simple_cache.hh"
    cxx_class  = "gem5::SimpleCache"

    cpu_side = VectorSlavePort("CPU Side Port, receive requests")
    mem_side = MasterPort("Mem Side Port, sends requests")

    latency = Param.Cycles(1,"Cycle taken on a hit or receives a miss")

    size = Param.MemorySize('1kB',"The size of the cache")

    system = Param.System(Parent.any,
    "The pointer to the system cache is part of")