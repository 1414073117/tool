local idl = require "sfidl"

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

local function doQuery(cfg, file, iPath)
    local schema, err = idl.loadSchema(os.getenv("CC_SCHEMA"), false)
    if not schema then
        return {}
    end

    local ccServ, err = schema:getServiceEngine("cfg.center", "rpc", false, 3)
    if not ccServ then
        return {}
    end

    local getData_list = {}
    local fd, err = ccServ.Open(os.getenv("CC_TENANT"), os.getenv("CC_DB"), {cfg}, schema:newObject("cc.mode", "r"), "sfvpn")
    if fd then
        local obj, err = ccServ.Query(fd, schema:newObject("cc.name", cfg), iPath)
        if obj then
            local resSchema = idl.loadSchema(file, false)
            local np, err = obj:convert(resSchema, cfg)
            if np then
                getData_list = np:getValue()
                if not getData_list or type(getData_list) ~= "table" then
                    getData_list = {}
                end
            end
        end
    end

    ccServ.Close(fd)
    return getData_list
end


local function occupy_qosClass(type, name)
    local qos_data = doQuery("net.qosClassList", "/sfos/system/schema/config/networkplatform/config.qosvlines.schema", '$')
    for _, qosClass in pairs(qos_data) do
        if qosClass and qosClass.match and qosClass.match.subifVlan then
            local subifVlan = qosClass.match.subifVlan
            if "SFVPN" == subifVlan.type and subifVlan.sfvpn then
                for _, tunnel in pairs(subifVlan.sfvpn) do
                    print(ToStringEx(tunnel))
                    if tunnel.type == type then
                        if type == "USER_MANAGER" and name == tunnel.user then
                            return true, qosClass.name
                        elseif type == "CONNECT_MANAGER" and name == tunnel.headquarters then
                            return true, qosClass.name
                        end
                    end
                end 
            end
        end
    end

    return false, nil
end

occupy_qosClass("CONNECT_MANAGER", "444_001")
occupy_qosClass("CONNECT_MANAGER", "444_002")
occupy_qosClass("USER_MANAGER", "XQC_001")
occupy_qosClass("USER_MANAGER", "XQC_002")
occupy_qosClass("sadasd", "XQC_002")