#!/bin/bash

repeat=$(ps -aux | grep $0 | wc -l)
old_deta=0

while [ $repeat -le 3 ];do
    new_deta=$(date '+%s')
    # echo $old_deta"->"$new_deta
    core_max=$(mpstat -P ALL -u 1 2 | grep "Average" | awk 'NR>=2{print $12}' | sort | awk 'NR<=1')
    # echo $core_max
    if [[ $(echo "$core_max < 10" | bc -l) -eq 1 && $(echo "$old_deta + 600 < $new_deta" | bc -l) -eq 1 ]]; then
        old_deta=$new_deta
        perf record -F 99 -a -g -o /sfdata/log/core/cpuPerf_${new_deta}.date sleep 10
    fi
done
