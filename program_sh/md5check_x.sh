#!/bin/bash

#升级前文件md5校验，升级前文件是否正常

declare -A md5list

function AF69_123_Md5list()
{
    md5list[/sfos/system/dp/lib/plugins/tcpproxy.so]="de707dfd6ae179cee037e6a1ecee8198"
    md5list[/sfos/system/dp/bin/ospfd]="7ca060b407982a4bc908723f493ec4b7"
    md5list[/sfos/system/dp/bin/dataplane]="16bf4d463139628bee038831dc32bf79,59934f64714a5db52ca26e9d212aa15a"
    md5list[/sfos/system/schema/cli/networkplatform/cli.sslproxyStatus.schema]="b1c3b8ed18e0cf02ff5ac5d87860b245"
    md5list[/sfos/config/sup/plugins/policy.so]="6d4e44c3fdbb9cf3b2756213bfd6b1d3,a3e619c4834f7044f3fe0e9715aee850"
    md5list[/sfos/system/lib64/libcc_core.so.1.0.1]="d7a13f7026c4a35d8dbc09f27f4eacae,c490025677a925c800e7a95538233722"
    md5list[/sfos/system/lib64/libtcpx.so.0.0.1]="6f2c029d604baf3904e27d1e8629e5de"
    md5list[/sfos/system/share/apiserver/plugins/strategy/sslproxyPolicy.lua]="3c958c3ac1cd6c7bdb8df9a547648ce3"
    md5list[/sfos/system/share/cfg-center/scripts/hasync/haInterfaceExt.lua]="e25795daced1be59cdf80e6d62dbe7eb"
    md5list[/sfos/system/dp/bin/snmpd]="7f21d40c44d95b676334784d1bbde6c3"
    md5list[/sfos/system/lib64/libnetsnmp.so.35.0.0]="b12bba7d2abdb6c57c6b7c41ad40e75f"
}

function AF75_506_Md5list()
{
    md5list[/sfos/system/dp/bin/dataplane]="89b57727098c1af18a929cefda2ae8af"
    md5list[/sfos/system/dp/lib/plugins/tcpproxy.so]="d7a95a998cd49a5af439189f50cd4d38"
    md5list[/sfos/system/dp/lib/plugins/sfvpn.so]="a0518ea56ee81e95efea9ad177025486"
    md5list[/sfos/config/sup/plugins/policy.so]="80c602e04a62ea1fb4c5876a6dea20b0"
    md5list[/sfos/system/lib64/libnetsnmp.so.35.0.0]="b12bba7d2abdb6c57c6b7c41ad40e75f"
    md5list[/sfos/system/lib64/libtcpx.so.0.0.1]="38dd406fc31fd3a457bf453eafaad7f8"
    md5list[/sfos/system/bin/nic_init.py]="62d87b3e36ca18641f4fd882a024ef1e"

}

function md5Contrast()
{
    filename=$1
    filemd5=$2

    if [[ -n ${filename} && -f ${filename} ]]; then
        filemd5_new=$(md5sum "${filename}" | awk '{print $1}')
    else
        echo "ERROR:文件"${filename}"不存在"
        return 1
    fi

    if [ -z ${filemd5} ]; then
        echo "ERROR:文件"${filename}"的md5"${filemd5}"为空"
        return 1
    fi

    if [ -z "${filemd5_new}" ]; then
        echo "ERROR:"${filename}"当前md5数值获取失败。"
        return 1
    else
        if [[ ${filemd5} == *${filemd5_new}* ]]; then
            echo "YES  :"${filename}"当前md5数值校验成功！！！"
            return 0
        else
            echo "ERROR:"${filename}"当前md5数值校验失败！！！"
            return 1
        fi
    fi

    return 0
}

function md5Check()
{
    declare -i type_x=0
    for key in ${!md5list[@]}
    do
        md5Contrast "${key}" "${md5list[$key]}"
        if [ $? -ne 0 ]; then
            type_x=1
        fi
    done

    if [ "$type_x" -eq 0 ]; then
        echo "YES       全部文件md5正常"
        return 0
    else
        echo "NO        出现异常 ERROR"
        return 1
    fi
}

function getFileMd5()
{
    file_x=$1
    if [[ -n ${file_x} && -f ${file_x} ]]; then
        exec < ${file_x}
        new_file=${file_x}".md5"
        :> ${new_file}
        while read line; do
            # echo ${line}
            if [[ -n ${line} && -f ${line} ]]; then
                md5sum "${line}" >> ${new_file}
            else
                echo "ERROR:文件"${line}"不存在已跳过"
            fi
        done
    else
        echo "ERROR:输入文件"${file_x}"不存在"
        return 1
    fi
}

function checkFileMd5()
{
    file_x=$1
    declare -i type_x=0

    if [[ -n ${file_x} && -f ${file_x} ]]; then
        exec < ${file_x}
        while read line; do
            if [ -n "${line}" ]; then
                filemd5=$(echo  ${line} | awk '{print $1}')
                filename=$(echo  ${line} | awk '{print $2}')
                md5Contrast "${filename}" "${filemd5}"
                if [ $? -ne 0 ]; then
                    type_x=1
                fi
            fi
        done
    else
        echo "ERROR:输入文件"${file_x}"不存在"
        return 1
    fi

    if [ "$type_x" -eq 0 ]; then
        echo "YES       全部文件md5正常"
        return 0
    else
        echo "NO        出现异常 ERROR"
        return 1
    fi
}

function getFileLsof()
{
    file_x=$1
    list_lsof=""
    if [[ -n ${file_x} && -f ${file_x} ]]; then
        exec < ${file_x}
        while read line; do
            if [[ -n ${line} && -f ${line} ]]; then
                pid_lsof=$(lsof ${line} | awk 'NR >=2{print $2}')
                if [[ $? == 0 && -n $pid_lsof ]]; then
                    list_lsof=$list_lsof" "$pid_lsof
                fi
            else
                echo "ERROR:文件"${line}"不存在已跳过"
            fi
        done

        if [[ -n $list_lsof ]]; then
            array=($(echo ${list_lsof} | sed 's/ /\n/g'| sort | uniq ))
            for element in ${array[@]}; do
                echo ${element}":"$(ps -o command -p ${element} | awk 'NR >=2{print $0}' )
            done
        fi
    else
        echo "ERROR:输入文件"${file_x}"不存在"
        return 1
    fi
}

function main()
{
    case $1 in
        AF69|af69)
            AF69_123_Md5list
            md5Check
        ;;
        AF75|af75)
            AF75_506_Md5list
            md5Check
        ;;
        NEWMD5|newmd5)
            getFileMd5 $2
        ;;
        CHECKMD5|checkmd5)
            checkFileMd5 $2
        ;;
        LSOF|lsof)
            getFileLsof $2
        ;;
        *)
            echo "输入格式："
            echo "    AF69        校验AF69源文件md5"
            echo "    AF75        校验AF75源文件md5"
            echo "    NEWMD5      根据文件产生md5文件"
            echo "    CHECKMD5    根据md5文件校验文件是否正确"
            echo "    LSOF        获取使用当前文件进程名"
        ;;
    esac
}

main "$1" "$2"