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


#!/bin/bash

mpstat_sleep=2
perf_sleep=2
perf_pathlog="/sfdata/log/work_core"
perf_counter=0
function perf_worker()
{
    local ret_1=0
    local cpu_list=$(mpstat -P ALL $mpstat_sleep 1 | grep 'Average' | awk 'NR > 2 && $12 < 10.0 {print $2"-"$12}')
    for cpu in $cpu_list; do
        echo "Do something with CPU $cpu"
        local cpu_number=$(echo $cpu | awk -F '-' '{print $1}')
        local cpu_oper=$(echo $cpu | awk -F '-' '{print $2}')
        perf record -C $cpu_number -F 99 -a -g -o ${perf_pathlog}/cpu${cpu_number}_${cpu_oper}_sleep${perf_sleep}_$(date "+%Y%m%d%H%M%S").date sleep $perf_sleep &
        ret_1=1
        ((perf_counter++))
    done
    return $ret_1
}

while [ $perf_counter -lt 1000 ]; do
    perf_worker
    if [ $? != 0 ]; then
        echo "sleep 60"
        sleep 60
    else
        echo "sleep 2"
        sleep 2
    fi
done


#!/bin/bash

# 设置进程名称和内存阈值
process_name="networkd"
memory_threshold=$((500*1024)) # 500MB in KB

# 获取进程ID
init_process_id=$(pgrep $process_name)

if [ -z "$init_process_id" ]; then
  echo "进程 $process_name 不存在"
  exit 1
fi

# 获取进程的初始内存使用量
initial_memory=$(ps -p $init_process_id -o rss=)

# 监控内存增长
while true; do
  process_id=$(pgrep $process_name)

  if [ -z "$process_id" ]; then
    echo "进程 $process_name 不存在"
    exit 0
  fi
  if [ $init_process_id -ne $process_id ]; then
    echo "进程 $process_name 已经重启"
    init_process_id=$process_id
    kill -55 $init_process_id
  fi

  current_memory=$(ps -p $process_id -o rss=)
  
  if [ "$current_memory" -gt "$memory_threshold" ]; then
    echo "进程 $process_name 内存使用超过阈值，正在杀死进程..."
    kill -55 $process_id
    sleep 10
    kill -9 $process_id
  fi

  sleep 30
done

