local idl = require "sfidl"
local log = require "logex"
local osdir = require "osdir"
local schemaFile = osdir.SCHEMA_DIR.."/apiserver/networkplatform/api.ha.schema"
local tenant = "public"
local localProgid = "local"

local schema, err = idl.loadSchema(schemaFile, false)
local serv, err = schema:getServiceEngine("apiserver.service", "rpc", false, 3)

local function getSfHAStatus()
    local ret, err = serv.getHaRoleState("", { namespace = tenant ,progid = localProgid})
    if err ~= nil or ret == nil then
        log.debug("", "getHaRoleState failed: ", string.format("%s", err))
        return true
    end

    local data = ret:getValue()
    print(data["role"])
    if "SLAVE" == data["role"]  then
        return false
    end

    return true
end

print(getSfHAStatus())