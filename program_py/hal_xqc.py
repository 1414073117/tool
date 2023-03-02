#! /usr/bin/env python3
#coding=utf-8

import re
import sys
import os
import string
from subhealth.collector.hardware.hal import HalUtils


def led_hh(ty_xy):
    # ret = serv.HalLedList()
    rpc_engine = HalUtils.get_hal_engine('rpc')
    if ty_xy == -1:
        memory_info = HalUtils.get_hal_info(rpc_engine, 'HalLedList')
        print(memory_info)
    if ty_xy == 1:
        memory_info = HalUtils.get_hal_info(rpc_engine, 'HalLedSet', name="-ha-led-plane-sangfor", mode=0xf1 , rgb=1)
        print(memory_info)
    
    if ty_xy == 0:
        memory_info = HalUtils.get_hal_info(rpc_engine, 'HalLedSet', name="-ha-led-plane-sangfor", mode=0xf1 , rgb=0)
        print(memory_info)
    
    if ty_xy == 2:
        memory_info = HalUtils.get_hal_info(rpc_engine, 'HalLedSet', name="-ha-led-plane-sangfor", mode=0xf2 , rgb=1)
        print(memory_info)


def bypass_hh(ty_xy):
    # ret = serv.HalLedList()
    rpc_engine = HalUtils.get_hal_engine('rpc')
    if ty_xy == 1:
        memory_info = HalUtils.get_hal_info(rpc_engine, 'HalBypassRuntime', state=True)
        print(memory_info)

    if ty_xy == 0:
        memory_info = HalUtils.get_hal_info(rpc_engine, 'HalBypassRuntime', state=False)
        print(memory_info)

    if ty_xy == 2:
        memory_info = HalUtils.get_hal_info(rpc_engine, 'HalBypassDisableboot')
        print(memory_info)

    if ty_xy == 3:
        memory_info = HalUtils.get_hal_info(rpc_engine, 'HalResetBypass')
        print(memory_info)


if __name__ == "__main__":
    led_hh(int(sys.argv[1]))