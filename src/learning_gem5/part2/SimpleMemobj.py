from m5.params import *
from m5.SimObject import SimObject
from m5.proxy import *#?


class SimpleMemobj(SimObject):
    type = 'SimpleMemobj'
    cxx_header = 'learning_gem5/part2/simple_memobj.hh'
    cxx_class  = 'gem5::SimpleMemobj'

    inst_port = SlavePort("CPU Side Port, receives requests")
    data_port = SlavePort("CPU Side Port, received requests")
    mem_side  = MasterPort("Memory Side Port, sends requests")
