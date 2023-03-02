#!/usr/bin/env lloader
print("000")
local OSROOT = os.getenv("OSROOT", "/sfos/system")
package.path = OSROOT .. "/lualibs/?.lua" .. ";" .. OSROOT .. "/share/iptables/scripts/?.lua"
local idl = require "sfidl"
local utils = require "utils"
local snmpUtils = require "snmpUtils"

print("000")

local tenant = "public"
local db = "running"

print("000")

local function xqc()
    -- 加载hal schema
    print("001")
    local schema, err = idl.loadSchema("/sfos/system/etc/schema/service/hal.schema", false)
    if err ~= nil then
        print(err)
    end

    -- 加载服务
    print("002")
    local impl, err = schema:getServiceImpls(idl.SERV_TYPE_LIB, "driver.service");
    if err ~= nil then
        print(err)
    end

    local ret, err = impl.HalPciInterfaceList()
    if err ~= nil then
        print(err)
    end
    
    print("003")

    if ret ~= nil then
        for i=1,#ret do
            print(i)
        end
    end
    return
end

print("000")

xqc()