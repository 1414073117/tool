#!/usr/bin/env lloader

local util     = require "utils"
local json     = require "cjson"
local jutil    = require "cjson.util"
local fs       = require "fs"
local log      = require "logex"
local filepath = require "filepath"
local parse    = require("argparse")("sfcli_template", "配置导出命令行自动化测试工具")
local jsSch    = require "jsonschema"
local M        = {osroot = os.getenv("OSROOT") or "/sfos/system"}
package.path   = string.format("%s;%s/lualibs/cli/?.lua", package.path,  M.osroot)
package.cpath  = string.format("%s;%s/lualibs/cli/?.so",  package.cpath, M.osroot)
local base     = require "export/base"

function ToStringEx(value)
    if type(value)=='table' then
       return TableToStr(value)
    elseif type(value)=='string' then
        return "\""..value.."\""
    else
       return tostring(value)
    end
end

function TableToStr(t)
    if t == nil then return "" end
    local retstr= "{"

    local i = 1
    for key,value in pairs(t) do
        local signal = ","
        if i==1 then
          signal = ""
        end

        if key == i then
            retstr = retstr..signal..ToStringEx(value)
        elseif type(key)=='number' then
            retstr = retstr..signal..'['..ToStringEx(key).."]:"..ToStringEx(value)
        elseif type(key) == 'string' then
            retstr = retstr..signal..ToStringEx(key)..":"..ToStringEx(value)
        elseif type(key)=='userdata' then
            retstr = retstr..signal.."*s"..TableToStr(getmetatable(key)).."*e"..":"..ToStringEx(value)
        else
            retstr = retstr..signal..key..":"..ToStringEx(value)
        end

        i = i+1
    end

    retstr = retstr.."}"
    return retstr
end

local unique_pz_whole = {}

function xqc_pz()
    local dir_x = io.popen("/sfos/system/bin/lloader /sfos/system/bin/cc_client.lua listSchema")
    local tab = util.split(dir_x:read("*all"), "\n")
    for i, file in ipairs(tab) do
        if not unique_pz_whole[file]  then
            print("qs:", file)
        else
            unique_pz_whole[file] = false
        end
    end

    for i, file in pairs(unique_pz_whole) do
        -- print(i, file)
        if file == true  then
            print("dy:", i)
        end
    end
end

function convert(v, pz_table)
    local unique = {}
    local tp  = require("export/template")
    local t, err = tp.PreProcce(v)
    if not t then
        print("load template fail: %s", err)
        return oe.EPERM, _T("Template preprocessing failure")
    end

    local cmds = t["cmds"]
    if cmds ~= nil then
        for i, cmd in ipairs(cmds) do
            -- print("cmd:", cmd)
            local cmd_tab = util.split(cmd, "[ ()=~]") 
            for _, node in ipairs(cmd_tab) do
                -- print("node:", node)
                if string.match(node, ":%$") then
                    local pz = util.split(node, ":")[1]
                    -- print("pz:", pz)
                    if not unique[pz] then
                        table.insert(pz_table, pz)
                        unique[pz] = true
                        unique_pz_whole[pz] = true
                    end
                end
            end
        end
    end
end

function traverse(path)
    local dir_x = io.popen("ls " ..  path)
    local tab = util.split(dir_x:read("*all"), "\n") 
    local file_size = 0
    for i, file in ipairs(tab) do
        if file ~= "." and file ~= ".." and string.match(file, "%.tp$") then
            -- print(file..":")
            file_size = file_size + 1
            local f = path..'/'..file
            local pz_table = {}
            convert(f, pz_table)
            print(file..": ", ToStringEx(pz_table))
        end
    end
    print("file size:", file_size)
    xqc_pz()
    print(ToStringEx(unique_pz_whole))
end

function main()
    parse:option("-t --template", "配置转换的模板")
    local args  = parse:parse()
    if not args.template then
        print("缺少必须参数template")
        print(parse:get_help())
        os.exit(1)
    end

    traverse(args.template)
end

main()