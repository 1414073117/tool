#!/bin/bash  
  
for((i=1;i<=1000;i++));  
do
    echo "while:"$i
    sfcli -login admin/admin -c "config; interface eth3.4094; ip address dhcp; zone L3_trust_C"
    sleep 1s
    sfcli -login admin/admin -c "config; interface eth3.4094; no zone; exit; no interface eth3.4094"
    sfcli -login admin/admin -c "config; interface veth.4094; ip address dhcp; zone L3_trust_C"
    sleep 1s
    sfcli -login admin/admin -c "config; interface veth.4094; no zone; exit; no interface veth.4094"
done