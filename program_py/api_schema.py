import base64
import time
import subprocess
import tempfile
import requests
import signal


headers = {'Content-Type': 'application/json'}

def sfcli(cmd):
    try:
        command = 'sfcli -c "%s"' %(cmd)
        print(command)
        subp = subprocess.Popen(command,shell=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE,encoding="utf-8")
        subp.wait(5)
        if subp.poll() == 0:
            return True, subp.communicate()[0]
        return False, None
    except:
        return False, None

def sfapi(uri, method='GET', post_data=None, batch=False):
    if batch == True:
        url = 'http://0.0.0.0:8085/api/batch/v1/namespaces/public/%s' %(uri)
    else:
        url = 'http://0.0.0.0:8085/api/v1/namespaces/public/%s' %(uri)
    if method == 'GET':
        resp = requests.get(url)
    elif method == 'POST':
        resp = requests.post(url, headers = headers, json = post_data)
    elif method == 'DELETE':
        resp = requests.delete(url)
    elif method == 'PUT':
        resp = requests.put(url, headers = headers, json = post_data)
    elif method == 'PATCH':
        resp = requests.patch(url, headers = headers, json = post_data)
    else:
        print('Unsupported method %s' %(method))
    if resp.status_code != 200:
        print('Status code error!')
        return False, None
    return True, resp.text

if __name__ == "__main__":
    print("001")
    # ret, body = sfapi("ripng/1")
    # print(ret, "  ", body)
    # ret, cont = sfcli("show ipv6 rip rout")
    # print(ret, "  ", cont)
    ret, body = sfapi("ripng/1/network")
    print(ret, "  ", body)
    netbody = [{"publishRoute":"3001::/112"},{"publishRoute":"3002::/112"}]
    ret, body = sfapi("ripng/1/network", "POST", netbody, True)
    print(ret, "  ", body)