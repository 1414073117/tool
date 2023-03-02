
local urlparser = require "socket.url"

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


local parse_url = urlparser.parse("https://www.baidu.com:865/sada/asds")
print(ToStringEx(parse_url))