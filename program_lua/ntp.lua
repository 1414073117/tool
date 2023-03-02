#!/usr/bin/env lloader

function ntp_sjian(TIMEOUT, s1, s2, s3)
    local serverTbl = { { server = s1}, { server = s2}, { server = s3},}

    local ntp_server_quantity = 0
    for _, oneServer in pairs(serverTbl) do
        if oneServer.server and oneServer.server ~= "" then
            ntp_server_quantity = ntp_server_quantity + 1
        end
    end
    if ntp_server_quantity >= 1 and TIMEOUT * ntp_server_quantity >= 60 then
        TIMEOUT = math.floor(60 / ntp_server_quantity)
    end

    --确定单个循环超时时间最小为5s
    if TIMEOUT < 5 then
        TIMEOUT = 5
    end

    print("server size："..tostring(ntp_server_quantity)..", time out: "..tostring(TIMEOUT).."s")
end

function toString()
    local x = math.random(1, 3)
    if x == 1 then
        return nil
    end

    if x == 2 then
        return ""
    end

    if x == 3 then
        return "s"
    end

    return -1
end

function main(siez_while)
    for i=1, siez_while do
        local num = math.random(0, 100)
        local s1, s2, s3 = toString(), toString(), toString()
        -- print("num："..tostring(num)..", s1: "..tostring(s1)..", s2: "..tostring(s2)..", s3: "..tostring(s3))
        ntp_sjian(num, s1, s2, s3)
    end
end

main(1000)