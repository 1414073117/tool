from dnslib import *
from dnslib.server import *

ddns_table = {}

def ddns_file_dns(file_url):
    with open(file_url, 'r') as file:
        content = file.read()
    lines = [line.strip() for line in content.split('\n')]
    for line in lines:
        if line:
            domain, ip = line.split('=')
            ddns_table[domain.strip()] = ip.strip()

def ddns_ipv4_resolve(request, qname, qtype):
    ddns_file_dns("ddns_dns.txt")
    reply = DNSRecord(DNSHeader(id=request.header.id, qr=1, aa=1, ra=1), q=request.q)
    name = str(qname).strip(".")
    if ddns_table.get(name):
        reply.add_answer(RR(qname, qtype, rdata=A(ddns_table[name])))
    return reply

def ddns_ipv6_resolve(request, qname, qtype):
    reply = DNSRecord(DNSHeader(id=request.header.id, qr=1, aa=1, ra=1), q=request.q)
    # reply.add_answer(RR(qname, qtype, rdata=AAAA('::1')))
    return reply

# DNS 解析器
class MyResolver(BaseResolver):
    def resolve(self, request, handler):
        qname = request.q.qname
        qtype = request.q.qtype

        if qtype == QTYPE.A:
            reply = ddns_ipv4_resolve(request, qname, qtype)
            return reply
        elif qtype == QTYPE.AAAA:
            reply = ddns_ipv6_resolve(request, qname, qtype)
            return reply
        else:
            reply = DNSRecord(DNSHeader(id=request.header.id, qr=1, aa=1, ra=1), q=request.q)
            reply.header.rcode = getattr(RCODE, 'NXDOMAIN')

        return reply

# 创建 DNS 服务器
resolver = MyResolver()
server = DNSServer(resolver, address="0.0.0.0", tcp=False)

# 启动 DNS 服务器
server.start()