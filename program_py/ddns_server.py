#! /usr/bin/env python3
# -*- coding:UTF-8 -*-
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs
import socket
import base64
import json
import ssl
import cgi
import datetime

host_ip = "192.168.1.2"

dns_server_host = {
    "members.dyndns.org" : host_ip,
    "dynupdate.noip.com" : host_ip,
    "dynamic.zoneedit.com" : host_ip,
    "api.cp.easydns.com" : host_ip,
    "setip.dynaccess.com" : host_ip,
    "www.duckdns.org" : host_ip,
    "freedns.afraid.org" : host_ip
}

user_pass = {
    "admin" : "admin",
    "xqc" : "xuqingchun"
}

def ddns_dns_server_file(fileName):
    with open(fileName, 'w') as file:
        for key, value in dns_server_host.items():
            line = f"{key} = {value}\n"
            file.write(line)

def ddns_parse_path(headers, s_path):
    params = {}
    parsed_url = urlparse(s_path)
    params = parse_qs(parsed_url.query)
    user = ''
    password = ''
    if "Authorization" in headers.keys():
        autho = headers["Authorization"]
        if autho.startswith("Basic "):
            autho = autho[6:]
            decoded_autho = base64.b64decode(autho).decode("utf-8")
            user, password = decoded_autho.split(":", 1)

    # print("001", parsed_url.path)
    # print("002", params)
    # print("003", user)
    # print("004", password)
    return True, user, password, parsed_url.path, params

def members_dyndns_org(user, password, params):
    if user in user_pass.keys() and password == user_pass[user]:
        if "hostname" in params.keys() and "myip" in params.keys():
            hostname = params["hostname"][0]
            myip = params["myip"][0]
            print(hostname)
            print(myip)
            dns_server_host[hostname] = myip
            ddns_dns_server_file("ddns_dns.txt")
            return 40, 'nochg'
        else:
            return 2, ''
    else:
        return 1, ''

def dynupdate_noip_com(user, password, params):
    return members_dyndns_org(user, password, params)

def setip_dynaccess_com(user, password, params):
    return members_dyndns_org(user, password, params)

def dynamic_zoneedit_com(user, password, params):
    if user in user_pass.keys() and password == user_pass[user]:
        if "host" in params.keys() and "dnsto" in params.keys():
            host = params["host"][0]
            dnsto = params["dnsto"][0]
            print(host)
            print(dnsto)
            dns_server_host[host] = dnsto
            ddns_dns_server_file("ddns_dns.txt")
            return 40, 'OK'
        else:
            return 2, ''
    else:
        return 1, ''

def api_cp_easydns_com(user, password, params):
    if user in user_pass.keys() and password == user_pass[user]:
        if "hostname" in params.keys() and "myip" in params.keys():
            hostname = params["hostname"][0]
            myip = params["myip"][0]
            print(hostname)
            print(myip)
            dns_server_host[hostname] = myip
            ddns_dns_server_file("ddns_dns.txt")
            return 40, 'OK'
        else:
            return 2, ''
    else:
        return 1, ''

def www_duckdns_org(user, password, params):
    if "token" in params.keys():
        token = params["token"][0]
        print(token)
        if "domains" in params.keys() and "ip" in params.keys():
            domains = params["domains"][0]
            ip = params["ip"][0]
            print(domains)
            print(ip)
            dns_server_host[domains] = ip
            ddns_dns_server_file("ddns_dns.txt")
            return 40, 'good'
        else:
            return 2, ''
    else:
        return 1, ''

def freedns_afraid_org(user, password, params):
    if "hostname" in params.keys() and "address" in params.keys():
        hostname = params["hostname"][0]
        address = params["address"][0]
        print(hostname)
        print(address)
        dns_server_host[hostname] = address
        ddns_dns_server_file("ddns_dns.txt")
        return 40, hostname + ':' + address
    else:
        return 2, ''


class TodoHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        print(self.headers)
        print(self.path)
        ddns_state = 1
        ddns_data = ''
        host = self.headers['host']
        ret, user, password, path, params = ddns_parse_path(self.headers, str(self.path))
        if ret == False:
            self.send_error(415, "Only json data is supported.")
            return

        print(path)
        if host == "members.dyndns.org" and path == '/nic/update':
            # length = int(self.headers['content-length'])  # 获取除头部后的请求参数的长度
            # datas = self.rfile.read(length) # 获取请求参数数据，请求数据为json字符串
            print("members_dyndns_org")
            ddns_state, ddns_data = members_dyndns_org(user, password, params)
        elif host == "dynupdate.noip.com" and path == '/nic/update':
            print("dynupdate_noip_com")
            ddns_state, ddns_data = dynupdate_noip_com(user, password, params)
        elif host == "setip.dynaccess.com" and path == '/nic/update':
            print("setip_dynaccess_com")
            ddns_state, ddns_data = setip_dynaccess_com(user, password, params)
        elif host == "dynamic.zoneedit.com" and path == '/auth/dynamic.html':
            print("dynamic_zoneedit_com")
            ddns_state, ddns_data = dynamic_zoneedit_com(user, password, params)
        elif host == "api.cp.easydns.com" and path == '/dyn/generic.php':
            print("api_cp_easydns_com")
            ddns_state, ddns_data = api_cp_easydns_com(user, password, params)
        elif host == "www.duckdns.org" and path == '/update':
            print("www_duckdns_org")
            ddns_state, ddns_data = www_duckdns_org(user, password, params)
        elif host == "freedns.afraid.org":
            print("freedns_afraid_org")
            if path == '/dynamic/update.php':
                ddns_state, ddns_data = freedns_afraid_org(user, password, params)
            elif path == '/api/':
                ddns_state = 11
            else:
                ddns_state = 10
        else:
            ddns_state = 10
        
        if ddns_state == 40:
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(ddns_data.encode('utf-8'))
        elif ddns_state == 1:
            self.send_error(401, "Wrong username and password")
        elif ddns_state == 2:
            self.send_error(405, "Unrecognized content")
        elif ddns_state == 11:
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write('www.qq2.com|10.10.10.10|https://freedns.afraid.org/dynamic/update.php?action=getdyndns'.encode('utf-8'))
        else:
            self.send_error(404, "Not Found")
        
        self.connection.shutdown(socket.SHUT_WR)


    def do_POST(self):
        ctype, pdict = cgi.parse_header(self.headers['content-type'])
        print(ctype, pdict)
        token = self.headers['X-Auth-Token']
        print(token)
        self.send_error(415, "Only json data is supported.")

class HTTPSServer(HTTPServer):
    def __init__(self, server_address, RequestHandlerClass, bind_and_activate=True, timeout=30):
        super().__init__(server_address, RequestHandlerClass, bind_and_activate)
        self.timeout = timeout

    def process_request(self, request, client_address):
        request.settimeout(self.timeout)
        super().process_request(request, client_address)

if __name__ == '__main__':
    server = HTTPSServer((host_ip,443), TodoHandler, timeout=10)
    server.socket = ssl.wrap_socket(server.socket, server_side=True, certfile='cert.pem', keyfile="key.pem", ssl_version=ssl.PROTOCOL_TLSv1)
    ddns_dns_server_file("ddns_dns.txt")
    server.debug = True
    server.serve_forever()