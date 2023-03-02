
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
# from turtle import st

def ipgroup(size_i, size_x):
    for i in range(0, size_i):
        domain_str = "sfcli -login admin/root123. -c \"config; ipgroup test" + str(i) + "; type domains; "
        for x in range(0, size_x):
            domain_str = domain_str + "domains www.test" + str(i) + "-" + str(x) + ".com; "
        domain_str = domain_str + "exit;\""
        print(domain_str)
        os.system(domain_str)


if __name__ == '__main__':
    # switch_info = {}
    # switch_info["001"] = "003"
    # with open('/sfdata/log/ripngd/xqc.txt',mode='a',encoding='utf-8') as f:
    #         f.write(datetime.datetime.now().strftime('%Y-%m-%d %H-%M-%S') + '\n')
    #         f.write(str(switch_info) + '\n')

    ipgroup(250, 200)