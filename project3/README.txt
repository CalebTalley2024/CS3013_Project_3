I'mAdd explanations here!


current Liveshare link: https://prod.liveshare.vsengsaas.visualstudio.com/join?F45BC38C07AFB31DDB8E3BDCC321B4FE22CF


Use this to run: make ./mm



Should MM_map by default make page writtable when using automap?

should I use CHECK for automapping


for page fautls
- only consider if valid = 0?

should I page fault for trying to store in a read only disk (eg. making a copy)

valid bit vs swapped bit

do I need "make_resident"

is it fine to not have virtual page number? (no virtual memory slot?)

virtual frame number = virtual page number?


make_resident, eject phys 1 pid 0 vp 0 pp 1

from what I can tell

phys 1: Refers to physical frame number 1.
pid 0: Refers to Process ID 0.
vp 0: Refers to Virtual Page number 0.
pp 1: Refers to Physical Page number 1.

arnt phys and pp the same?