import os
import time
from scapy.all import *
from optparse import OptionParser
from threading import Thread


def genIP(rule):  
    ip, maskLen = rule.split('/')
    parts = ip.split('.')
    
    # 补全ip，如 10.24 => 10.24.0.0
    while(len(parts) < 4):
        parts.append('0')
    
    # 将IP字符串转化成整数
    ipNum = 0
    for i in parts:
        ipNum = 256*ipNum + int(i)

    # 求出子网的起始和结束位置
    subnetLen = 32 - int(maskLen)
    assert(subnetLen > 1 and subnetLen <= 32)
    ipStart = ipNum >> subnetLen << subnetLen   # 子网地址 
    ipEnd = ipStart + 2**(subnetLen) - 1  # 广播地址 
    
    # 整数转化成IP
    def IPNum2str(num):
        result = []
        while num >= 256:
            result.append(num % 256)
            num = num // 256
        result.append(num)
        result.reverse()
        return '.'.join([str(i) for i in result])
    
    import random
    return IPNum2str(random.randint(ipStart+1, ipEnd-1))

def scan(ip):
    # print(ip)
    packet = Ether(src="fe:fc:fe:48:05:19", dst="fe:fc:fe:61:30:70")/IP(src=ip, dst="192.168.2.2")/ICMP()
    sendp(packet, iface='ens19')


def main():
    start_time = time.time()
    i = 100000
    while(i > 0):
        scan(genIP("0.0.0.0/0"))
        # i = i - 1
    end_time = time.time()
    print("函数耗时:",end_time-start_time)
    # parser = OptionParser("Usage program -i <target host> -n <website> -p <target port>")
    # parser.add_option("-i", '--host', type="string",dest="tgtIP",help="specify target host or website")
    # parser.add_option("-n","--network", type="string",dest="tgtNetwork",help="specify target Network")
    # parser.add_option("-f", "--addressfile", type="string", dest="tgtFile", help="specify target addressfile")
    # parser.add_option("-p","--port", type="string",dest="tgtPorts",help="specify target port separated by comma")
    # options,args = parser.parse_args()

    # tgtIP = options.tgtIP
    # tgtNetwork = options.tgtNetwork
    # tgtFile = options.tgtFile
    # tgtPorts = options.tgtPorts
    # tgtPorts = tgtPorts.split(",")

    # if tgtPorts is None or tgtNetwork is None and tgtIP is None and tgtFile is None :
    #     print(parser.usage)
    #     exit(0)


if __name__ == '__main__':
    main()