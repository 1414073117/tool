from json.tool import main
import sys
import time
import os
import psutil
import argparse

def obtain_name_pid(pname):
    for proc in psutil.process_iter():
        if pname == proc.name():
            return proc.pid
    print("process ID acquisition failed, process name:" + pname)
    exit(0)
    return -1

def get_linux_mem():
    memory_svm = psutil.virtual_memory()
    return memory_svm

def get_pid_mem(pid):
    p=psutil.Process(pid)
    mem_rss = p.memory_info().rss
    # print(p.memory_info())
    mem_percent = p.memory_percent()
    return {"rss" : mem_rss, "percent" : mem_percent}

def file_header(process):
    str_data = '       now time     '
    str_data = str_data + '%13s'%("total")
    str_data = str_data + '%13s'%("used")
    str_data = str_data + '%13s'%("free")
    str_data = str_data + '%13s'%("shared")
    str_data = str_data + '%13s'%("buffers")
    str_data = str_data + '%13s'%("cached")
    str_data = str_data + '%13s'%("available")
    for name, pid in process.items():
        str_data = str_data + '%13s'%(name)
    str_data = str_data + '\n'
    return str_data

def file_content(process):
    memory_svm = get_linux_mem()
    str_data = time.strftime('%Y-%m-%d %H:%M:%S ', time.localtime(time.time()))
    str_data = str_data + '%13d'%(memory_svm.total)
    str_data = str_data + '%13d'%(memory_svm.used)
    str_data = str_data + '%13d'%(memory_svm.free)
    str_data = str_data + '%13d'%(memory_svm.shared)
    str_data = str_data + '%13d'%(memory_svm.buffers)
    str_data = str_data + '%13d'%(memory_svm.cached)
    str_data = str_data + '%13d'%(memory_svm.available)
    for name, pid in process.items():
        pid_mem_list = get_pid_mem(pid)
        str_data = str_data + '%13d'%(pid_mem_list["rss"])
        # str_data = str_data + '%13f'%(pid_mem_list["percent"])
    str_data = str_data + '\n'
    return str_data

def cyclic_acquisition_write_file_mem(frequency, interval, fileName, process):
    file_w = open(fileName,'w')
    file_w.write(file_header(process))
    for i in range(frequency + 1):
        file_w.write(file_content(process))
        time.sleep(interval)
    file_w.close()

def cyclic_acquisition_write_mem(frequency, interval, process):
    print(file_header(process))
    for i in range(frequency + 1):
        print(file_content(process))
        time.sleep(interval)

def argparse_cmd():
    parser = argparse.ArgumentParser()
    parser.add_argument('-n', '--name',type = str, help = "monitoring process name \",\"division  ripngd,ospfd")
    parser.add_argument('-q', '--quantity', type = int, default = 600, help = "obtain quantity     600")
    parser.add_argument('-i', '--interval', type = int, default = 1, help = "obtain interval(s)     1")
    parser.add_argument('-f', '--file',type = str, help = "Save file  mem.txt")
    parser.add_argument('-d', '--debug', action="store_true", help = "debug")
    args = parser.parse_args()
    process_list = {}
    if args.name != None:
        for name in args.name.split(','):
            process_list[name] = obtain_name_pid(name)
    
    if args.debug == True:
        print("process", str(process_list))
        print("quantity", args.quantity)
        print("interval", args.interval)
        print("file", args.file)
    return args.quantity, args.interval, args.file, process_list

if __name__ == "__main__":
    frequency, interval, fileName, process = argparse_cmd()
    if fileName != None:
        cyclic_acquisition_write_file_mem(frequency, interval, fileName, process)
    else:
        cyclic_acquisition_write_mem(frequency, interval, process)
