import base64
import time
import subprocess
import tempfile
import requests
import signal
import json
import os

def read_cron_json(file_name):
    with open(file_name, 'r') as f:
        data = json.load(f)
    # print(data)
    return data

def del_cron_json(data):
    for item in data:
        ml = "sfcrontab uninstall --name \"" + item["name"] + "\""
        print(ml)
        os.system(ml)

def del_cron_json(data):
    for item in data:
        ml = "sfcrontab uninstall --name \"" + item["name"] + "\""
        print(ml)
        os.system(ml)

def add_cron(size_max):
    for i in range(1, size_max + 1):
        ml = "sfcrontab install -g \"af\" -n \"xqc" + str(i) + "\" -u \"root\" -M \"*\" -H \"*\" -D \"*\" -m \"*\" -w \"*\" -e \"date >> /root/xqc.txt\""
        print(ml)
        os.system(ml)

def sleep_cpu():
    while True:
        time.sleep(0.1)

if __name__ == "__main__":
    data = read_cron_json("/sfos/system/etc/sfcrond/sfcron.json")
    del_cron_json(data)
    # add_cron(100)
    sleep_cpu()