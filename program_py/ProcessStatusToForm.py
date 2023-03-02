from json.tool import main
import sys
import time
import os
import psutil
import xlwt

def obtain_name_pid(pname):
    for proc in psutil.process_iter():
        if pname == proc.name():
            return proc.pid
    print("进程id获取失败，进程名字：" + pname)
    exit(0)
    return -1

def get_linux_mem(prtype):
    memory_svm = psutil.virtual_memory()
    if prtype == 1:
        print(memory_svm)
    # memory_table = {"total" : memory_svm.total, "available" : memory_svm.available,
    #     "percent" : memory_svm.percent, "used" : memory_svm.used,
    #     "free" : memory_svm.free, "active" : memory_svm.active,
    #     "inactive" : memory_svm.inactive, "buffers" : memory_svm.buffers,
    #     "cached" : memory_svm.cached, "shared" : memory_svm.shared}
    # print(memory_table)
    return memory_svm

def cyclic_acquisition_write_file_mem(c_quantity, c_interval, filename):
    workbook = xlwt.Workbook(encoding = 'utf-8')
    worksheet = workbook.add_sheet(filename)
    for i in range(c_quantity + 1):
        memory_svm = get_linux_mem(0)
        worksheet.write(i, 0, label = memory_svm.total)
        worksheet.write(i, 1, label = memory_svm.used)
        worksheet.write(i, 2, label = memory_svm.free)
        worksheet.write(i, 3, label = memory_svm.shared)
        worksheet.write(i, 4, label = memory_svm.buffers)
        worksheet.write(i, 5, label = memory_svm.cached)
        worksheet.write(i, 6, label = memory_svm.available)
        worksheet.write(i, 7, label = memory_svm.percent)
        worksheet.write(i, 8, label = memory_svm.active)
        worksheet.write(i, 9, label = memory_svm.inactive)
        workbook.save(filename)
        time.sleep(c_interval)

def get_cpu_mem(pid, prtype):
    p=psutil.Process(pid)
    cpu_percent = p.cpu_percent(interval=0.1)
    mem_percent = p.memory_percent()
    mem_rss = p.memory_info().rss
    cur_time = time.strftime('%Y-%m-%d  %H:%M:%S', time.localtime(time.time()))
    if prtype == 1:
        print("pid:", pid)
        print("time:", cur_time)
        print("cpu:", cpu_percent)
        print("mem:", mem_percent)
        print("rss:", mem_rss)
    return {"cur_time" : cur_time, "cpu_percent" : cpu_percent, "mem_percent" : mem_percent}

def cyclic_acquisition(c_quantity, c_interval, pid):
    for i in range(c_quantity + 1):
        get_cpu_mem(pid, 1)
        time.sleep(c_interval)

def cyclic_acquisition_write_file(c_quantity, c_interval, pid, filename):
    workbook = xlwt.Workbook(encoding = 'utf-8')
    worksheet = workbook.add_sheet(filename)
    for i in range(c_quantity + 1):
        info_table = get_cpu_mem(pid, 0)
        worksheet.write(i, 0, label = info_table["cur_time"])
        worksheet.write(i, 1, label = info_table["cpu_percent"])
        worksheet.write(i, 2, label = info_table["mem_percent"])
        workbook.save(filename)
        time.sleep(c_interval)

if __name__ == "__main__":
    pid = obtain_name_pid("ripngd")
    get_cpu_mem(pid, 1)
    # cyclic_acquisition(5000, 1, pid)
    # cyclic_acquisition_write_file(60*60, 1, pid, "xqc.xls")
    # get_linux_mem()
    # cyclic_acquisition_write_file_mem(60*60, 1, "xqc.xls")
