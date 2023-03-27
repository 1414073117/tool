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
local isQuiet  = false

pluginDir = "/share/cli/plugin/"

xqc_blm = 0

-- 连接字符串
function StrCon(...)
    local tb, cnt = {}, select("#", ...)
    if cnt == 0 then
        return ""
    end

    for i = 1, cnt do
        local v = select(i, ...)
        if v and v ~= "" then
            v = util.trim(tostring(v))
            table.insert(tb, v)
        end
    end

    return util.trim(table.concat(tb, " "))
end

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

local function XQC_blmz()
    str_xqc = "bl"..ToStringEx(xqc_blm)
    xqc_blm = xqc_blm + 1
    return str_xqc
end

-- 使用通用的正则表达式，将字符串分隔
local function GetRegular(name, s, p)
    if not string.find(s, name) then
        return nil
    end

    local pattern = p or "(.*)"..name.."[%s]*%([%s]*([%w_%$%s%.%:,=%-]+)[%s]*%)(.*)"
    local result  = {}
    string.gsub(s, pattern, function(...)
        result =  {...}
    end)

    return unpack(result)
end

local function GetNoRelation(cfgs, relation)
    for k, _ in pairs(relation) do
        return k
    end

    return cfgs[1]
end

local function cvtNeeded(cmd, field)
    -- 不需要转换单个字段，全部都转换
    if not field then
        return true
    end

    -- 命令前面有`+`号，表示为模式命令，都需要转换
    if string.find(cmd, "^[%s]*%+") then
        return true
    end

    -- 如果该命令，包含该字段的变量，那么就需要转换该变量
    if string.find(cmd, field) then
        return true
    end

    return false
end

function cvtPlug(template)
    if not template.plugin then
        return {}
    end

    -- 如果用户指定的目录下没有脚本，再尝试去默认目录中获取
    local file = filepath.Join(pluginDir, template.plugin)
    local mod = "EXPORT_LOAD_PLUGIN"

    local ok, res = pcall(dofile, file)
    if not ok then
        print("load plugin %s fail: %s", file, tostring(res))
        return {}
    end

    return res
end

local function ReverseFind(str, key)
    local k, s = key:reverse(), str:reverse()
    local b, e = string.find(s, k)
    if not b then
        return nil
    end

    return (#s+1-e), (#s+1-b)
end


local function GetExpress(key, str)
    local b, e = ReverseFind(str, key)
    if not b then
        return nil
    end

    local flag = 0
    local r1, r2, r3 = "", "", ""

    r1 = string.sub(str, 1, b-1)
    for i = e+1, #str do
        local c = string.sub(str, i, i)
        if c == "(" then
            flag = flag + 1
        elseif c == ")" then
            flag = flag - 1
        end

        if flag == 0 then
            r2 = string.sub(str, e+2, i-1)
            r3 = string.sub(str, i+1)
            break
        end
    end

    return r1, r2, r3
end

local function isInBracket(prefix, suffix)
    return (prefix:find("{")) and (suffix:find("}"))
end

local function GetArgs(str)
    local result = {}
    local flag = false
    local idx  = 1

    for i = 1, #str do
        local c = string.sub(str, i, i)
        if c == "(" then
            flag = true
        elseif c == ")" then
            flag = false
        end

        if (c == ",") and (not flag) then
            table.insert(result, string.sub(str, idx, i-1))
            idx = i + 1
        end
    end

    if idx < #str then
        table.insert(result, string.sub(str, idx))
    end

    return result
end

local function compare(exp, userdata, tb)
    local r1, r2, eq = exp, nil, true
    if string.find(exp, "==") then
        r1, r2 = GetRegular("==", exp, "%s*(.-)%s*==%s*(.+)%s*")
    elseif string.find(exp, "~=") then
        eq = false
        r1, r2 = GetRegular("~=", exp, "%s*(.-)%s*~=%s*(.+)%s*")
    end

    table.insert(tb, r1)
    if r2 then
        table.insert(tb, r2)
    end

    return 1
end

-- 处理select下的表达式 `&&` `||`
local function selectExp(exp, userdata, tb)
    print(exp)
    tb = tb or {}
    local function express(delim)
        print(exp)
        local b, e = string.find(exp, delim)
        if not b then
            return nil
        end

        local r1 = selectExp(string.sub(exp, 1, b-1), userdata, tb)
        local r2 = selectExp(string.sub(exp, e+1), userdata, tb)
        if delim == "&&" then
            return StrCon(r1, "and", r2)
        end
        return StrCon(r1, "or", r2)
    end

    local ret = express("&&")                        -- 处理与的情况
    if ret == nil then
        ret = express("||")                          -- 处理或的情况
        if ret == nil then
            compare(exp, userdata, tb)   -- 处理单独的表达式
            ret = exp
        end
    end

    return ret, tb
end

local function evaluation(str, arg)
    local r = string.gsub(str, "(.*)%$(%d+)(.*)", function(a1, a2, a3)
        local num = tonumber(a2)
        if not num or num + 1 > #arg then
            print("模板表达式的选值错误 error_xqc")
            return ""
        end

        return base.Trim(a1..tostring(arg[num+1])..a3)
    end)

    if r == str then
        return str
    end

    return evaluation(r, arg)
end



local function xqc_print(str, userdata)
    print(str)
    return str, 1
end

local function Parse(str, userdata)
    -- SELECT函数，二选一
    local function Select(str, userdata)
        print("str",ToStringEx(str))
        local r1, r2, r3 = GetExpress("SELECT", str)
        if not r1 or isInBracket(r1, r3) then
            return str, 0
        end

        print("r1",ToStringEx(r1))
        print("r2",ToStringEx(r2))
        print("r3",ToStringEx(r3))

        if r1 ~= nil and r1 ~= "" then
            Parse(r1, userdata)
        end
        local t = GetArgs(r2)
        print("xqwc",ToStringEx(t))
        local r, arg = selectExp(t[1], userdata)
        print("if ", r, "then")
        local x1 = evaluation(t[2] or "", arg)
        print("else")
        local x2 = evaluation(t[3] or "", arg)
        print("end")

        if r2 ~= nil and r2 ~= "" then
            Parse(r2, userdata)
        end
        return str, 1
    end

    local funcs = {
        Select,
        -- Tolower,
        -- Array,
        -- Func,
        -- GetRef,
        -- DelBracket,
        -- SetConfVal,
        -- PassWord,
        xqc_print
    }

    for _, f in pairs(funcs) do
        str , ert = f(str, userdata)
        if erf == 1 then
            break
        end
    end

    return str
end

function Convert_(template, field, skipDep)
    -- 查看是否有插件，有插件需要加载插件
    local ud = cvtPlug(template)
    print("ud:", ToStringEx(ud))

    -- local output = ud.output
    local modeOk = 0   -- 是否生成模式命令，0 没有模式，1 有模式，但没有生成模式命令， 2 有模式，并且生成模式命令

    for _, v in ipairs(template.cmds or {}) do
        local result, isMode = "", v:find("^%s*%+")
        if modeOk == 0 and isMode then
            modeOk = 1
        end
        print("v:", ToStringEx(v))
        print("modeOk:", ToStringEx(modeOk))
        print("result:", ToStringEx(result))

        if cvtNeeded(v, field) then 
            if isMode or modeOk == 2 or modeOk == 0 then
                result = Parse(v, ud)
            end
        end

        if result ~= "" and isMode then
            modeOk = 2
        end

        -- 只有再没有模式或者已经生成了模式命令时，才会生成其他命令
        if modeOk == 0 or modeOk == 2 then
            print(result, template)
        end
    end

    print("end\n")
   return true, oe.SUCCESS
end

local function ConfigForeach(tpl)
    local relation = tpl.relation or {}
    local cfg = GetNoRelation(tpl.cfgs, relation)
    local each = tpl.foreach or {cfg..":$$"}


    for _, v in ipairs(each) do
        print("v:", ToStringEx(v))
        local tb, cnt = util.split(v, ":")
        local reverse, cfgName, field = false, tb[1], tb[2]
        if cnt < 2 then
            dbg("模板中的foreach字段，需要包括配置和变量(%s)", v)
            return oe.ENOENT, _F("Template file exception")
        end

        if cnt == 3 then
            reverse = (tb[1] == "reverse")
            cfgName, field = tb[2], tb[3]
        end

        print("reverse:", ToStringEx(reverse))
        print("cfgName:", ToStringEx(cfgName))
        print("field:", ToStringEx(field))

        return Convert_(tpl, nil, true)
    end

    return oe.SUCCESS
end

function convert(args)
    local tp  = require("export/template")
    local dep = require("export/dep")
    tpls = args.template
    if type(tpls) == "string" then
        tpls = {tpls}
    end

    for _, v in ipairs(tpls) do
        local t, err = tp.PreProcce(v)
        if not t then
            print("load template fail: %s", err)
            return oe.EPERM, _T("Template preprocessing failure")
        end

        local ret, err = ConfigForeach(t)
        if ret ~= oe.SUCCESS then
            return ret, err
        end
    end
end

function main()
    parse:option("-t --template", "配置转换的模板")
    local args  = parse:parse()
    if not args.template then
        print("缺少必须参数template")
        print(parse:get_help())
        os.exit(1)
    end

    convert(args)
end

main()