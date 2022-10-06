from m5.params import *
from m5.SimObject import SimObject

class HelloObject(SimObject):
    type = 'HelloObject'
    #convention to have type same as cpp class
    cxx_header = "learning_gem5/part2/hello_object.hh"
    cxx_class  = "gem5::HelloObject"