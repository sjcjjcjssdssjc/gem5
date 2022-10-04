import m5
from m5.objects import *
system = System()
#father of all
system.clk_domain = SrcClockDomain()
system.clk_domain.clk = '1GHz'
system.clk_domain.voltage_domain = VoltageDomain()

system.mem_mode = 'timing'
system.mem_ranges = 'AddrRange('512MB')
system.cpu = TimingSimpleCPU()
system.membus = SystemXBar()

system.cpu.icache_port = system.membus.cpu_side_ports
system.cpu.dcache_port = system.membus.cpu_side_ports
#connect the cache port directly to mem because we have no cache.
#Another notable kind of magic of the = of two ports in a gem5 Python
#configuration is that, it is allowed to have one port on one side, and
#an array of ports on the other side

