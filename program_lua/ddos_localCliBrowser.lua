local browsingGids = {
    "net.ddosLocalRules",
    "net.localRule",
    "net.localRule.enable",
    "net.localRule.scanDefend",
    "net.localRule.scanDefend.portScan",
    "net.localRule.scanDefend.portScan.enable",
    "net.localRule.scanDefend.portScan.threshold",
    "net.localRule.scanDefend.portScan.blockTime",
    "net.localRule.scanDefend.portScan.resetTimeOut",
    "net.localRule.scanDefend.portScan.deleteTimeOut",
    "net.localRule.scanDefend.holeScan",
    "net.localFloodCfg",
    "net.syncFlood",
    "net.syncFlood.enable",
    "net.syncFlood.perDstIpDropThreshold",
    "net.syncFlood.srcIpBlockThreshold",
    "net.syncFlood.blockTime",
    "net.syncFlood.perIpActiveThreshold",
    "net.wanFlood",
    "net.wanFlood.enable",
    "net.wanFlood.perDstIpDropThreshold",
    "net.wanFlood.srcIpBlockThreshold",
    "net.wanFlood.blockTime",
    "net.wanFlood",
    "net.wanFlood.enable",
    "net.wanFlood.perDstIpDropThreshold",
    "net.wanFlood.srcIpBlockThreshold",
    "net.wanFlood.blockTime",
    "net.wanFlood",
    "net.wanFlood.enable",
    "net.wanFlood.perDstIpDropThreshold",
    "net.wanFlood.srcIpBlockThreshold",
    "net.wanFlood.blockTime",
    "net.action",
    "net.action.log",
    "net.action.deny"
}

local function ddosLocalRulesCliX_wanFlood(cmds, wanFlood, prop_str, defalist)
    if wanFlood:getField("srcIpBlockThreshold") ~= nil then
        if wanFlood:getField("srcIpBlockThreshold"):getValue() ~= defalist[0] then
            table.insert(cmds, string.format("%s source-threshold %d", prop_str, wanFlood:getField("srcIpBlockThreshold"):getValue()))
        end
    end

    if wanFlood:getField("blockTime") ~= nil then
        if wanFlood:getField("blockTime"):getValue() ~= defalist[1] then
            table.insert(cmds, string.format("%s block-time %d", prop_str, wanFlood:getField("blockTime"):getValue()))
        end
    end

    if wanFlood:getField("perDstIpDropThreshold") ~= nil then
        if wanFlood:getField("perDstIpDropThreshold"):getValue() ~= defalist[2] then
            table.insert(cmds, string.format("%s destination-threshold %d", prop_str, wanFlood:getField("perDstIpDropThreshold"):getValue()))
        end
    end

    if wanFlood:getField("enable") ~= nil then
        if wanFlood:getField("enable"):getValue() ~= true then
            table.insert(cmds, string.format("no %s", prop_str))
        end
    end

    return true
end

local function ddosLocalRulesCliX_synFlood(cmds, syncFlood, prop_str, defalist)
    if syncFlood:getField("srcIpBlockThreshold") ~= nil then
        if syncFlood:getField("srcIpBlockThreshold"):getValue() ~= defalist[0] then
            table.insert(cmds, string.format("%s source-threshold %d", prop_str, syncFlood:getField("srcIpBlockThreshold"):getValue()))
        end
    end

    if syncFlood:getField("blockTime") ~= nil then
        if syncFlood:getField("blockTime"):getValue() ~= defalist[1] then
            table.insert(cmds, string.format("%s block-time %d", prop_str, syncFlood:getField("blockTime"):getValue()))
        end
    end

    if syncFlood:getField("perDstIpDropThreshold") ~= nil then
        if syncFlood:getField("perDstIpDropThreshold"):getValue() ~= defalist[2] then
            table.insert(cmds, string.format("%s destination-threshold %d", prop_str, syncFlood:getField("perDstIpDropThreshold"):getValue()))
        end
    end

    if syncFlood:getField("perIpActiveThreshold") ~= nil then
        if syncFlood:getField("perIpActiveThreshold"):getValue() ~= defalist[3] then
            table.insert(cmds, string.format("%s proxy-threshold %d", prop_str, syncFlood:getField("perIpActiveThreshold"):getValue()))
        end
    end

    if syncFlood:getField("enable") ~= nil then
        if syncFlood:getField("enable"):getValue() ~= true then
            table.insert(cmds, string.format("no %s", prop_str))
        end
    end

    return true
end

local function ddosLocalRulesCliX(obj, ctx)
    assert(obj)

    local localRule = obj:getField("localRule")
    local cmds = {}
    table.insert(cmds, "config")
    table.insert(cmds, "ddos-defense local")
    if localRule ~= nil then
        local action = localRule:getField("action")
        if action:getField("log") ~= nil then
            if action:getField("log"):getValue() == true then
                table.insert(cmds, "action log")
            else
                table.insert(cmds, "no action log")
            end
        end

        if action:getField("deny") ~= nil then
            if action:getField("deny"):getValue() == true then
                table.insert(cmds, "action deny")
            else
                table.insert(cmds, "no action deny")
            end
        end

        local portScan = localRule:getField("scanDefend"):getField("portScan")
        if portScan:getField("threshold"):getValue() ~= 120 then
            table.insert(cmds, string.format("port-scan threshold %d", portScan:getField("threshold"):getValue()))
        end

        if portScan:getField("blockTime"):getValue() ~= 300 then
            table.insert(cmds, string.format("port-scan block-time %d", portScan:getField("blockTime"):getValue()))
        end

        if portScan:getField("resetTimeOut"):getValue() ~= 20 then
            table.insert(cmds, string.format("port-scan reset-timeout %d", portScan:getField("resetTimeOut"):getValue()))
        end

        if portScan:getField("deleteTimeOut"):getValue() ~= 300 then
            table.insert(cmds, string.format("port-scan remove-timeout %d", portScan:getField("deleteTimeOut"):getValue()))
        end

        if portScan:getField("enable"):getValue() ~= true then
            table.insert(cmds, "no port-scan")
        end

        ddosLocalRulesCliX_synFlood(cmds, localRule:getField("ddosDefend"):getField("syncFlood"), "syn-flood", {10000, 300, 10000, 135})
        ddosLocalRulesCliX_wanFlood(cmds, localRule:getField("ddosDefend"):getField("dnsFlood"), "dns-query-flood", {1000, 300, 10000})
        ddosLocalRulesCliX_wanFlood(cmds, localRule:getField("ddosDefend"):getField("udpFlood"), "udp-flood", {1000, 300, 10000})
        ddosLocalRulesCliX_wanFlood(cmds, localRule:getField("ddosDefend"):getField("icmpFlood"), "icmp-flood", {1000, 300, 2000})

        
        if localRule:getField("enable"):getValue() == true then
            table.insert(cmds, "enable")
        else
            table.insert(cmds, "disable")
        end
    end

    local localPort, err = ctx:getConfig("net.ddosPortList")
    if err then
        return "", err
    end

    local cnt = localPort:getLength()
    for i = 0, cnt - 1 do
        local item = localPort:getItem(i)
        local g_protocol = "tcp"
        local g_portType = ""
        if item:getField("protocol"):getValue() == 1 then
            g_protocol = "tcp"
        elseif item:getField("protocol"):getValue() == 2 then
            g_protocol = "udp"
        end

        if item:getField("portType"):getValue() == 1 then
            g_portType = ""
        elseif item:getField("portType"):getValue() == 2 then
            g_portType = " ignore"
        end

        local range = item:getField("range")
        if range:getField("end") == nil then
            table.insert(cmds, string.format("port protocol %s portrange %d%s", g_protocol, range:getField("start"):getValue(), g_portType))
        else
            table.insert(cmds, string.format("port protocol %s portrange %d-%d%s", g_protocol, range:getField("start"):getValue(), range:getField("end"):getValue(), g_portType))
        end
    end
    table.insert(cmds, "end\n")

    return table.concat(cmds, "\n")
end

CfgCenter.regConfigBrowser("CLI", "net.ddosLocalRules", browsingGids, ddosLocalRulesCliX)