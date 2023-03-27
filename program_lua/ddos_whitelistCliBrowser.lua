local browsingGids = {
    "net.ddosIpExclude",
    "net.ddosIpExclude.list",
    "ipv46Address"
}

local function ddosIpExclude(obj, ctx)
    local cmds = {}
    local cnt = obj:getField("list"):getLength()
    table.insert(cmds, "config")
    for i = 0, cnt - 1 do
        local item = obj:getField("list"):getItem(i)
        table.insert(cmds, string.format("ddos-defense whitelist ip %s", item:getValue()))
    end

    local ddosByPass, err = ctx:getConfig("net.ddosByPass")
    if err then
        return "", err
    end

    if ddosByPass:getField("bypass"):getValue() == true then
        table.insert(cmds, "ddos-defense bypass")
    end

    table.insert(cmds, "end\n")
    return table.concat(cmds, "\n")
end

CfgCenter.regConfigBrowser("CLI", "net.ddosIpExclude", browsingGids, ddosIpExclude)