# !/usr/bin/python3
# -*- coding: utf-8 -*-
# @Time    :2021/3/1 18:48
# @Author  : TangYong
# @File    : api_cgi_config.py
# @Software: PyCharm

import re
import os
import uuid
import time
import random
import logging
import datetime
import requests

from logging.handlers import RotatingFileHandler
from concurrent.futures import ThreadPoolExecutor

requests.packages.urllib3.disable_warnings()


# ------------------------------登录/退出-----------------------------------
class LoginLogout:
    '''
    cgi/api方式登录和退出
    获取session，token
    '''
    def __init__(self,url,user,pwd):
        self.url = url
        self.user = user
        self.pwd = pwd
        self.token = None
        self.php_session = None
        self.headers = {
            'Content-Type': 'application/json;charset=UTF-8', 'Cookie': None}
        self.cookies = {
            'Content-Type': 'application/json;charset=UTF-8', 'SESSID': None}

    def get_php_session(self):
        '''
          获取前端请求的php session
          '''

        rsp = requests.get(
            url=f'{self.url}/captcha.php',
            verify=False
        )

        if rsp:
            self.php_session = rsp.headers['Set-Cookie']

        else:
            exit('获取前端php_session失败，请检查环境是否连通')

    def cgi_login(self):
        '''
        cgi方式登录控制台
        返回值供其cgi方式登录的调用
        :return: self.headers,self.token
        '''

        # 获取登录session
        self.get_php_session()
        self.headers['Cookie'] = self.php_session

        rsp = requests.post(
            url=f'{self.url}/api/v1/namespaces/@namespace/webuilogin',
            json={
                "name": self.user, "password": self.pwd, "__extdata": {"privacyEnable": True}},
            verify=False,
            headers=self.headers
        )

        if rsp:
            if rsp.json()['code'] == 0:

                # 获取登录控制台的session
                sessions = rsp.headers['Set-Cookie']

                # 将控制台的session和标识前端请求的session进行组合
                self.headers["Cookie"] = self.headers["Cookie"] + f";{sessions};showRobotTip=1"

                # 获取token
                self.token = rsp.json()['data']['cftoken']

                return self.headers,self.token

            else:
                exit(f'获取登录控制台session失败：{rsp.text}')
        else:
            exit(f'登录失败：{rsp.text}')

    def cgi_logout(self):
        '''
        cgi方式退出控制台
        '''

        # 前端请求/登录认证session
        php_session = ''
        auth_session = ''

        # 提取前端请求和前端登录标识session
        ck = self.headers['Cookie']

        # 获取前端请求session标识
        php_session_info = re.findall('PHPSESSID=\w+', ck)
        if php_session_info:
            php_session = php_session_info[0].split('=')[1]

        else:
            exit('未获取到前端请求session标识')

        # 获取前端登录session标识
        auth_session_info = re.findall('SESSID=\w{60,}', ck)
        if auth_session_info:
            auth_session = auth_session_info[0].split('=')[1]

        else:
            exit('未获取到前端登录session标识')

        rsp = requests.post(
            url=f'{url}/api/v1/namespaces/@namespace/logout',
            json={},
            verify=False,
            cookies={
                'PHPSESSID': php_session,
                'SESSID': auth_session,
            },
            headers={
                'Content-Type': 'application/json;charset=UTF-8', '_cftoken': self.token}
        )

        if rsp:
            if rsp.json()['code'] != 0:
                logger.error(f'cgi退出失败：{rsp.json()}')
        else:
            logger.error(f'cgi退出失败：{rsp.json()}')

    def api_login(self):

        '''
         api登录认证
          返回值供其api方式调用使用
        :return:self.cookies
        '''
        response = requests.post(
            url=url + '/api/v1/namespaces/public/login',
            # url= f'{self.url}/api/v1/namespaces/@namespace/login',
            json={
                "name": self.user,
                "password": self.pwd,
            },
            verify=False,
            cookies=self.cookies
        )

        if response.json():
            if response.json()['code'] == 0:
                token = response.json()['data']['loginResult']['token']
                self.cookies['SESSID'] = token
                return self.cookies
            else:
                logger.error(f'登录失败:{response.text}')
                exit(f'登录失败:{response.text}')
        else:
            logger.error(f'登录失败:{response.text}')
            exit(f'登录失败:{response.text}')

    def api_logout(self):
        '''
        api方式退出
        :return:
        '''

        response = requests.post(
            # url=f"{self.url}/api/v1/namespaces/public/logout",
            url=f"{self.url}/api/v1/namespaces/@namespace/logout",
            json={
                "loginResult": {
                    "token": self.cookies["SESSID"]
                }
            },
            verify=False,
            cookies=self.cookies,
        )

        if response.json():
            if response.json()['code'] != 0:
                logger.error(f'API退出失败：{response.json()}')
        else:
            logger.error(f'API退出失败：{response.json()}')


#日志记录工具
class Logger(logging.Logger):
    def __init__(self):
        super().__init__("log")
        self.setLevel(log_level)
        fmt = '%(asctime)s %(filename)s %(funcName)s [line:%(lineno)d] %(levelname)s %(message)s'
        fh = logging.Formatter(fmt)
        log_path = generate_log_file()
        if log_path:
            file_handle = RotatingFileHandler(log_path,encoding="UTF-8",maxBytes=1024*1024*10, backupCount=10)
            file_handle.setLevel(log_level)
            file_handle.setFormatter(fh)
            self.addHandler(file_handle)
        sh = logging.StreamHandler()
        sh.setFormatter(fh)
        sh.setLevel(log_level)
        self.addHandler(sh)


# ------------------------------公共函数-----------------------------------


#vlan接口
def add_del_vlan_interface_bak(i):
    '''
     添加和删除vlan口
     :param:

     '''

    #有区域
    zones_data = {
            "interfaces": {"vlanId": i, "name": f"veth.{i}", "ifType": "VLANIF", "description": "", "vsys": ["public"],
                           "reverseRouteEnable": False, "ipv4": {"ipv4Mode": "DHCP", "staticIp": []},
                           "ipv6": {"ipv6Param": {"enable": False, "mtu": 1500}, "ipv6Mode": "STATIC", "staticIp": []},
                           "mtu": 1500,
                           "bandSwitch": {"egressbandSwitch": {"value": 0}, "ingressbandSwitch": {"value": 0}},
                           "manage": {"https": True, "ping": True, "snmp": False, "ssh": False}},
            "zones": {"name": "L3_trust_A", "forwardType": "ROUTE", "interfaces": [f"veth.{i}"]}}

    #无区域
    no_zones_data = {
    "interfaces": {"vlanId": i, "name": f"veth.{i}", "ifType": "VLANIF", "description": "", "vsys": ["public"],
                   "reverseRouteEnable": False, "ipv4": {"ipv4Mode": "STATIC", "staticIp": []},
                   "ipv6": {"ipv6Param": {"enable": False, "mtu": 1500}, "ipv6Mode": "STATIC", "staticIp": []},
                   "mtu": 1500, "bandSwitch": {"egressbandSwitch": {"value": 0}, "ingressbandSwitch": {"value": 0}},
                   "manage": {"https": True, "ping": True, "snmp": False, "ssh": False}},
    "vsyszones": [{"vsysname": "public", "zonename": {}}]}

    v6_dhcp_zones_data = {
        "interfaces": {"vlanId": i, "name": f"veth.{i}", "ifType": "VLANIF", "description": "", "vsys": ["public"],
                       "reverseRouteEnable": False, "ipv4": {"ipv4Mode": "DHCP", "staticIp": [],
                                                             "dhcp": {"gateway": False, "unicast": False, "dns": True}},
                       "ipv6": {"ipv6Param": {"enable": True, "mtu": 1500}, "ipv6Mode": "DHCP6", "staticIp": [],
                                "dhcp6": {"clientMode": "ADDR"}}, "mtu": 1500,
                       "bandSwitch": {"egressbandSwitch": {"value": 0}, "ingressbandSwitch": {"value": 0}},
                       "manage": {"https": True, "ping": True, "snmp": False, "ssh": False}},
        "zones": {"name": "L3_trust_C", "forwardType": "ROUTE", "interfaces": [f"veth.{i}"]}
    }




    resp = requests.post(
        url=url+f'/api/v1/namespaces/{vs_name}/interfacemidware/add',

        json=v6_dhcp_zones_data,

        verify=False,
        headers={
            "Content-Type": "application/json",
        },
        cookies=cookies
    )

    rep_res = str(resp.content, 'utf-8')
    if rep_res:
        if str(resp.json()['code']).strip() == str(0):
            logger.info(f'添加vlan口成功:{rep_res}')
        else:
            logger.error(f'添加vlan口失败:{rep_res}')
    else:
        logger.error(f'添加vlan口失败:{rep_res}')

    time.sleep(wait_time)

    save_disk()


#子接口
def add_del_sub_interface(num):
    '''
     添加和删除子接口
     :param:
     '''
    # num = 100

    v6_dhcp_bak = {
        "interfaces": {"name": f"{network_card}.{num}", "vlanId": num, "subif": {"fatherInterface": network_card},
                       "ifType": "SUBIF",
                       "description": "", "vsys": ["public"], "reverseRouteEnable": False,
                       "ipv4": {"ipv4Mode": "DHCP", "staticIp": [], "dhcp": {"gateway": False, "dns": True}},
                       "ipv6": {"ipv6Param": {"enable": True, "mtu": 1500}, "ipv6Mode": "DHCP6", "staticIp": [],
                                "dhcp6": {"clientMode": "ADDR"}}, "mtu": 1500,
                       "bandSwitch": {"egressbandSwitch": {"value": 0}, "ingressbandSwitch": {"value": 0}},
                       "manage": {"https": True, "ping": True, "snmp": False, "ssh": False}},
        "vsyszones": [{"vsysname": "public", "zonename": {}}]
    }
    v4_static = {
            "interfaces": {"name": f"{network_card}.{num}" , "vlanId": num, "subif": {"fatherInterface": network_card},
                           "ifType": "SUBIF", "description": "", "reverseRouteEnable": False,
                           "ipv4": {"ipv4Mode": "STATIC", "staticIp": [], "dhcp": {"gateway": False}},
                           "ipv6": {"ipv6Mode": "STATIC", "staticIp": [],
                                    "ipv6Param": {"mtu": 1500, "enable": False}}, "mtu": 1500,
                           "manage": {"https": True, "ping": True, "ssh": False}}}
    v6_dhcp = {
        "interfaces": {"name": f"{network_card}.{num}", "vlanId": num, "subif": {"fatherInterface": f"eth{network_card}"}, "ifType": "SUBIF",
                       "description": "", "vsys": ["public"], "reverseRouteEnable": False,
                       "ipv4": {"ipv4Mode": "STATIC", "staticIp": []},
                       "ipv6": {"ipv6Param": {"enable": True, "mtu": 1500}, "ipv6Mode": "DHCP6", "staticIp": [],
                                "dhcp6": {"clientMode": "ADDR"}}, "mtu": 1500,
                       "bandSwitch": {"egressbandSwitch": {"value": 0}, "ingressbandSwitch": {"value": 0}},
                       "manage": {"https": True, "ping": True, "snmp": False, "ssh": False}},
        "zones": {"name": "L3_trust_C", "forwardType": "ROUTE", "interfaces": [f"eth{network_card}.{num}"]}}



    add_sub_interface_resp = requests.post(
        url=url+'/api/v1/namespaces/public/interfacemidware/add',
        json=v4_static,
        verify=False,
        headers={
            "Content-Type": "application/json",
        },
        cookies=cookies
    )

    add_sub_interface_res = str(add_sub_interface_resp.content, 'utf-8')
    if add_sub_interface_res:
        if str(add_sub_interface_resp.json()['code']).strip() == str(0):
            logger.info(f'添加子接口成功:{add_sub_interface_res}')
        else:
            logger.error(f'添加子接口失败:{add_sub_interface_res}')
    else:
        logger.error(f'添加子接口失败:{add_sub_interface_res}')

    time.sleep(wait_time)

    # del_resp = requests.post(
    #     url=url + '/api/v1/namespaces/public/interfacemidware/del',
    #     json=[
    #         {"interfaces": {"uuid": "E0F6F2C8B17349B2A7913166FD534E13", "name": f"{network_card}.{num}", "description": "",
    #                         "mtu": 1500, "ifType": "SUBIF", "ifMode": "ROUTE",
    #                         "ipv4": {"ipv4Mode": "DHCP", "staticIp": [], "dhcp": {"gateway": False}},
    #                         "ipv6": {"ipv6Mode": "STATIC", "staticIp": [],
    #                                  "ipv6Param": {"enable": False, "mtu": 1500}}, "reverseRouteEnable": False,
    #                         "manage": {"ssh": False, "https": True, "ping": True}, "vlanId": 42,
    #                         "subif": {"fatherInterface": network_card}}, "zones": {}, "pppoes": {}, "virtuallines": {}}],
    #     verify=False,
    #     headers={
    #         "Content-Type": "application/json",
    #     },
    #     cookies=cookies
    # )
    #
    #
    # del_sub_interface_res = str(del_resp.content, 'utf-8')
    # if del_sub_interface_res:
    #     if del_resp.json()['code'] == 0:
    #         logger.info(f'删除子接口成功:{del_sub_interface_res}')
    #     else:
    #         logger.error(f'删除子接口失败:{del_sub_interface_res}')
    # else:
    #     logger.error(f'删除子接口失败:{del_sub_interface_res}')
    #
    # time.sleep(wait_time)
    # save_disk()


if __name__ == '__main__':

    #日志层级，默认为info.10:debug;20:info;30:warning;40:error;50:critical
    log_level_list = [10,20,30,40,50]
    log_level = log_level_list[1]

    # 日志打印工具
    logger = Logger()


    # 设备地址、账号、密码
    url = 'https://10.90.7.1'
    user = 'admin'
    pwd = 'singfor123.'

    #登录，获取系统的session和token
    login_auth = LoginLogout(url,user,pwd)
    headers, token = login_auth.cgi_login()
    cookies = login_auth.api_login()

    #操作配置间隔时间(秒)
    wait_time = 1


    #一个未被使用的物理网卡
    network_card = 'eth1'

    #  public 表示根系统，其他为子系统
    vs_name = 'public'

    #是否对配置进行新增和删除
    is_add_del = False


    # 执行api/cgi配置下发,,默认每个api/cig只执行1次,如要长时间执行，传入int类型的数字，如:600,表示执行600秒
    # main(180000)

    for i  in range(1500,1600):
       add_del_vlan_interface_bak(i)
       add_del_sub_interface(i)
