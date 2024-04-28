-- 将IP地址和子网掩码转换为32位二进制数

local function snmp_to_broadcast(ip, mask)
    local ip_list = {}
    for substr in string.gmatch(ip, "([^%.]+)") do
        table.insert(ip_list, tonumber(substr))
    end
    
    local mask_list = {}
    for substr in string.gmatch(mask, "([^%.]+)") do
        table.insert(mask_list, tonumber(substr))
    end

    print(mask_list, ip_list)
    if #ip_list ~= 4 or #mask_list ~= 4 then
        return "IP conversion failed", false
    end

    local broadcast_list = {}
    for i = 1, 4 do
        table.insert(broadcast_list, bit.bor(bit.bxor(mask_list[i], 0xFF), ip_list[i]))
    end

    return table.concat(broadcast_list, "."), true
end

local ip = "65.95.32.6"
local mask = "255.255.0.0"
local broadcast_list = snmp_ip_mask_to_broadcast(ip, mask)
print(ip, mask, broadcast_list)
