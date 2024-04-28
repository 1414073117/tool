import os
import re
import time
import ipaddress

user_pass = "admin/admin"
dns_query_file = "/sfdata/log/core/dns.txt"
whitelist_sfcli = f"sfcli -login {user_pass} -c 'show whitelist'"
dns_sfcli = f"sfcli -login {user_pass} -s {dns_query_file}"

def identify_file(filename, string_list):
    with open(filename, 'w', encoding='utf-8') as file:
        for string in string_list:
            file.write(f"show ip hosts {string}" + '\n')

def identify_string(s):
    try:
        ipaddress.ip_address(s)
        return 1, "IP地址"
    except ValueError:
        pass

    try:
        ipaddress.ip_network(s, strict=False)
        return 2, "网段"
    except ValueError:
        pass

    if re.match(r'^([a-z0-9]+(-[a-z0-9]+)*\.)+[a-z]{2,}$', s, re.IGNORECASE):
        return 3, "域名"

    return 4, "未知格式"

def system_os_command(cmd_sfcli):
    process = os.popen(cmd_sfcli)
    output_lines = process.readlines()
    exit_code = process.close()
    if exit_code is None:
        return output_lines
    else:
        print(f"命令执行失败，退出码：{exit_code},运行命令：{cmd_sfcli}")
    return {}


def extract_domains(lines):
    data_start_index = None
    for i, line in enumerate(lines):
        if line.startswith('url'):
            data_start_index = i + 2
            break

    if data_start_index is None:
        return 1, "错误：找不到白名单数据的起始位置。"

    domains = []
    for line in lines[data_start_index:]:
        if line.startswith('='):
            break
        parts = line.split()
        if parts:
            domains.append(parts[0])

    commands = []
    for domain in domains:
        ret_dns, ret_str = identify_string(domain)
        # print(domain, ret_dns, ret_str)
        if ret_dns >= 3:
            commands.append(domain)

    return 0, commands

ret_x,command_list = extract_domains(system_os_command(whitelist_sfcli))

if ret_x == 0:
    identify_file(dns_query_file, command_list)
    for line in system_os_command(dns_sfcli):
        print(line, end='')
else:
    print(command_list)