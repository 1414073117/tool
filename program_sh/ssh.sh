#!/bin/bash

function random()
{
    min=0;
    max=65534;
    num=$(date +%s+%N);
    ((retnum=num%max+min));
    echo $retnum
    return $retnum;
}

exit 0

while ((1))
do
    random
    port=$?
    # retdata=$(curl http://192.168.2.2:$port)
    echo $port
    sleep 30s
done