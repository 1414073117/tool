#!/usr/bin/env lloader

SCRIPTPATH = os.getenv("SFVPN_ROOT_PATH") or "/sf/sgp/sfvpn"
package.path = SCRIPTPATH.."/lualibs/?.lua;"..package.path
package.cpath = SCRIPTPATH.."/lualibs/?.so;"..package.cpath

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

local function sfvpnLoadConfiguration()
    local success_xml, xml = pcall(require, "sfvpn_xml")
    if not success_xml or not xml then
        return false, nil, nil
    end

    local success_conf_common, conf_common = pcall(require, "config.conf_common")
    if not success_conf_common or not conf_common then
        return false, nil, nil
    end

    return true, xml, conf_common
end

local function MultiDataCenter_getAll(connInfo)
    local success, xml, conf_common = sfvpnLoadConfiguration()
    if not success or success == false then
        return -1, "load configuration failed"
    end

    local CONF_PATH = "/etc/sinfor/multiDc/MultiDataCenter.xml"
    local ret, xmlConf = pcall(xml.load, CONF_PATH)
    if not ret or not xmlConf then
        return -1, "load conf failed"
    end

    local xmlConfFilecontent = xmlConf:find("filecontent")
    if not xmlConfFilecontent then
        return -1, "filecontent does not exist "
    end

    local _, connectManagerList = xmlConfFilecontent:children("ConnectManager")
    for _, connectManager in pairs(connectManagerList or {}) do
        local oneSangfor = conf_common.confToTable(connectManager)
        if oneSangfor and oneSangfor.Connect then
            for _,connect in pairs(oneSangfor.Connect) do
                local center = {MainPool = {}, BackupPool = {}}
                if connect.ConnName then
                    center.ConnName = connect.ConnName
                end

                if connect.UserName then
                    center.UserName = connect.UserName
                end

                if connect.MainPool and #connect.MainPool == 1 and connect.MainPool[1].Server then
                    for _,value_2 in pairs(connect.MainPool[1].Server) do
                        if value_2.Name then
                            table.insert(center.MainPool, value_2.Name)
                        end
                    end
                end

                if connect.BackupPool and #connect.BackupPool == 1 and connect.BackupPool[1].Server then
                    for _,value_2 in pairs(connect.BackupPool[1].Server) do
                        if value_2.Name then
                            table.insert(center.BackupPool, value_2.Name)
                        end
                    end
                end

                table.insert(connInfo, center)
            end
        end
    end

    return 0, "success", connInfo
end

local function connectManager_getAll(connInfo)
    local success, xml, conf_common = sfvpnLoadConfiguration()
    if not success or success == false then
        return -1, "load configuration failed"
    end

    local CONF_PATH = conf_common.vpnconf.connectManager
    local ret, xmlConf = pcall(xml.load, CONF_PATH)
    if not ret or not xmlConf then
        return -1, "load conf failed"
    end

    local xmlConfFilecontent = xmlConf:find("filecontent")
    if not xmlConfFilecontent then
        return -1, "filecontent does not exist "
    end

    local _, connectManagerList = xmlConfFilecontent:children("ConnectManager")
    for _, connectManager in pairs(connectManagerList or {}) do
        local oneSangfor = conf_common.confToTable(connectManager)
        if oneSangfor and oneSangfor.source and oneSangfor.source ~= "multi"then
            local center = {}
            if oneSangfor.UserName then
                center.UserName = oneSangfor.UserName
            end

            if oneSangfor.name then
                center.tunName = oneSangfor.name
            end

            table.insert(connInfo, center)
        end
    end

    return 0, "success", connInfo
end

local function userManager_getAll(connInfo)
    local success, xml, conf_common = sfvpnLoadConfiguration()
    if not success or success == false then
        return -1, "load configuration failed"
    end

    local CONF_PATH = conf_common.vpnconf.userManager
    local ret, xmlConf = pcall(xml.load, CONF_PATH)
    if not ret or not xmlConf then
        return -1, "load conf failed"
    end

    local _, filecontentList = xmlConf:children("filecontent")
    for _, filecontent in pairs(filecontentList or {}) do
        local oneSangfor = conf_common.confToTable(filecontent)
        if oneSangfor and oneSangfor.User then
            for _,connect in pairs(oneSangfor.User) do
                local center = {}
                if connect.name then
                    center.UserName = connect.name
                end

                table.insert(connInfo, center)
            end
        end
    end

    return 0, "success", connInfo
end

local function ipsec_getAll(connInfo)
    local success, xml, conf_common = sfvpnLoadConfiguration()
    if not success or success == false then
        return -1, "load configuration failed"
    end

    local CONF_PATH = conf_common.vpnconf.ipsec
    local ret, xmlConf = pcall(xml.load, CONF_PATH)
    if not ret or not xmlConf then
        return -1, "load conf failed"
    end

    local xmlConfFilecontent = xmlConf:find("filecontent")
    if not xmlConfFilecontent then
        return -1, "filecontent does not exist "
    end

    local xqc_001, equipmentList = xmlConfFilecontent:children("equipment")
    for _, equipment in pairs(equipmentList or {}) do
        local oneSangfor = conf_common.confToTable(equipment)
        if oneSangfor then
            local center = {}
            if oneSangfor.name then
                center.tunName = oneSangfor.name
            end

            table.insert(connInfo, center)
        end
    end

    return 0, "success", connInfo
end

function main()
    local sfvpn_table = {}
    local xqc = {}
    MultiDataCenter_getAll(sfvpn_table)
    connectManager_getAll(sfvpn_table)
    userManager_getAll(sfvpn_table)
    ipsec_getAll(sfvpn_table)
    print(ToStringEx(sfvpn_table))
end

main()