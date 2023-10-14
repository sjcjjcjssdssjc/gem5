import m5
from m5.objects import *
root = Root(full_system = False)
root.hello = HelloObject()
root.hello.goodbye_object = GoodbyeObject(buffer_size = '100B',
write_bandwidth = '100MB/s')
#how is it initialized?

#Now, you can instantiate the HelloObject you created.
#All you need to do is call the Python “constructor”.
m5.instantiate()
print('m5 is instantiated')
exit_event = m5.simulate()
print('Exiting tick{} because of{}'
      .format(m5.curTick(), exit_event.getCause()))