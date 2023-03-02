
import sys
import os
import json
import codecs
import subprocess
import time
import math
import shutil
import ctypes
import re
import logging
import configparser
import multiprocessing
import glob
import copy
import datetime

def ip_to_int(ip_str):
    i = 0
    ip_int = 0
    for ip_part in ip_str.split('.')[::-1]:
        ip_int = ip_int + int(ip_part) * 256**i
        i += 1
    return ip_int

def int_to_ip(ip_int):
    s = []
    for i in range(4):
        ip_part = str(ip_int % 256)
        s.append(ip_part)
        ip_int //= 256
    return '.'.join(s[::-1])


def ipgroup(xqc_size):
    xqc_strdate = ""
    single_time = 100
    while xqc_size > 0:
        for i in range(0, single_time):
            if xqc_size > 0:
                str_single = "ipgroup "+str(10000 + xqc_size)+"; type ip; ipentry "+int_to_ip(ip_to_int("10.10.10.1") + xqc_size)
                xqc_size = xqc_size - 1

def interface_vlan(xqc_size):
    starting_point = 1
    for i in range(starting_point, starting_point + xqc_size):
        str_single = "interface veth."+str(i)+"; exit;"
        print(str_single)

def vsys(xqc_size):
    starting_point = 1
    for i in range(starting_point, starting_point + xqc_size):
        str_single = "vsys x"+str(i)+"; exit;"
        print(str_single)

def v_interface(xqc_size):
    starting_point = 1
    for i in range(starting_point, starting_point + xqc_size):
        str_single = "interface eth2."+str(i)+"; exit;"
        print(str_single)

def v_link_detect(xqc_size):
    starting_point = 0
    discriminator = 1
    for i in range(0, xqc_size):
        str_single = "link-detect ipv4 xqc"+ str(starting_point).zfill(3)
        str_single = str_single + "; detect-bfd local-address " + int_to_ip(ip_to_int("192.168.2.1") + starting_point) + "; detect-bfd local-discriminator " + str(discriminator)
        str_single = str_single + "; detect-bfd peer-address " + int_to_ip(ip_to_int("192.168.0.1") + starting_point) + "; detect-bfd remote-discriminator " + str(discriminator + 1)
        str_single = str_single + "; exit;"
        print(str_single)
        starting_point = starting_point + 1
        discriminator = discriminator + 2

def v_link_detectv6(xqc_size):
    starting_point = 0
    discriminator = 1
    for i in range(0, xqc_size):
        str_single = "link-detect ipv6 xqc"+ str(starting_point).zfill(3)
        str_single = str_single + "; detect-bfd local-address 2001:1::" + hex(starting_point + 1)[2:] + "; detect-bfd local-discriminator " + str(discriminator)
        str_single = str_single + "; detect-bfd peer-address 2001:2::2; detect-bfd remote-discriminator " + str(discriminator + 1)
        str_single = str_single + "; exit;"
        print(str_single)
        starting_point = starting_point + 1
        discriminator = discriminator + 2


if __name__ == '__main__':
    v_link_detectv6(256)