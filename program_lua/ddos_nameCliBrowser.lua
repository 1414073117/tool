local browsingGids = {
    "net.ddosRules",
    "net.ruleItem",
    "regexpCreator",
    "regexpUuid",
    "regexpName",
    "regexpDescription",
    "net.ruleItem.enable",
    "net.ruleItem.src",
    "net.ddosZone",
    "net.ddosZone.items",
    "net.ddosIpGroup",
    "net.ddosIpGroup.items",
    "net.ruleItem.scanDefend",
    "net.scanFlood",
    "net.scanFlood.enable",
    "net.scanFlood.threshold",
    "net.scanFlood.blockTime",
    "net.scanFlood",
    "net.scanFlood.enable",
    "net.scanFlood.threshold",
    "net.scanFlood.blockTime",
    "net.ruleItem.ruleType",
    "net.ddosLan",
    "net.lanFloodCfg",
    "net.lanFlood",
    "net.lanFlood.enable",
    "net.lanFlood.srcIpBlockThreshold",
    "net.lanFlood.blockTime",
    "net.lanFlood",
    "net.lanFlood.enable",
    "net.lanFlood.srcIpBlockThreshold",
    "net.lanFlood.blockTime",
    "net.lanFlood",
    "net.lanFlood.enable",
    "net.lanFlood.srcIpBlockThreshold",
    "net.lanFlood.blockTime",
    "net.lanFlood",
    "net.lanFlood.enable",
    "net.lanFlood.srcIpBlockThreshold",
    "net.lanFlood.blockTime",
    "net.lanFlood",
    "net.lanFlood.enable",
    "net.lanFlood.srcIpBlockThreshold",
    "net.lanFlood.blockTime",
    "net.ddosLan.attackDefend",
    "net.ddosLan.attackDefend.packetAttackDefend",
    "uint32",
    "net.ddosWan",
    "net.arpFlood",
    "net.arpFlood.enable",
    "net.arpFlood.ifThresh",
    "net.wanFloodCfg",
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
    "net.attackDefend",
    "net.attackDefend.packetAttackDefend",
    "uint32",
    "net.attackDefend.ipv4OptAttackDefend",
    "uint32",
    "net.attackDefend.tcpAttackDefend",
    "uint32",
    "net.action",
    "net.action.log",
    "net.action.deny",
    "net.ruleItem.position"
}

local function ddosRulesCliX_scanFlood(cmds, scanFlood, prop_str, defalist)
    if scanFlood:getField("threshold") ~= nil then
        if scanFlood:getField("threshold"):getValue() ~= defalist[0] then
            table.insert(cmds, string.format("%s threshold %d", prop_str, scanFlood:getField("threshold"):getValue()))
        end
    end

    if scanFlood:getField("blockTime") ~= nil then
        if scanFlood:getField("blockTime"):getValue() ~= defalist[1] then
            table.insert(cmds, string.format("%s block-time %d", prop_str, scanFlood:getField("blockTime"):getValue()))
        end
    end

    if scanFlood:getField("enable") ~= nil then
        if scanFlood:getField("enable"):getValue() ~= true then
            table.insert(cmds, string.format("no %s", prop_str))
        end
    end

    return true
end

local function ddosRulesCliX_lanFlood(cmds, lanFlood, prop_str, defalist)
    if lanFlood:getField("srcIpBlockThreshold") ~= nil then
        if lanFlood:getField("srcIpBlockThreshold"):getValue() ~= defalist[0] then
            table.insert(cmds, string.format("%s source-threshold %d", prop_str, lanFlood:getField("srcIpBlockThreshold"):getValue()))
        end
    end

    if lanFlood:getField("blockTime") ~= nil then
        if lanFlood:getField("blockTime"):getValue() ~= defalist[1] then
            table.insert(cmds, string.format("%s block-time %d", prop_str, lanFlood:getField("blockTime"):getValue()))
        end
    end

    if lanFlood:getField("enable") ~= nil then
        if lanFlood:getField("enable"):getValue() ~= true then
            table.insert(cmds, string.format("no %s", prop_str))
        end
    end

    return true
end

local function ddosRulesCliX_wanFlood(cmds, wanFlood, prop_str, defalist)
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

local function ddosRulesCliX_synFlood(cmds, syncFlood, prop_str, defalist)
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

local function ddosRulesCliX_packetAttackDefend(cmds, packetAttackDefend)
    local packet_cnt = packetAttackDefend:getLength()
    local pack_opt = {[1] = "unknown-proto", [2] = "tear-drop", [3] = "ip-fragment", [4] = "land-attack", [5] = "winnuke", [6] = "ip-directed-broadcast", [7] = "ping-of-death"}
    for i = 0, packet_cnt - 1 do
        table.insert(cmds, string.format("%s", pack_opt[packetAttackDefend:getItem(i):getValue()]))
    end

    return true
end

local function ddosRulesCliX_ipv4OptAttackDefend(cmds, ipv4OptAttackDefend)
    local ipv4_cnt = ipv4OptAttackDefend:getLength()
    local ipv4_opt = {[1] = "error", [2] = "time-stamp", [3] = "sec", [4] = "sid", [5] = "rr", [6] = "lsrr", [7] = "ssrr"}
    for i = 0, ipv4_cnt - 1 do
        table.insert(cmds, string.format("ip-option %s", ipv4_opt[ipv4OptAttackDefend:getItem(i):getValue()]))
    end

    return true
end

local function ddosRulesCliX_tcpAttackDefend(cmds, tcpAttackDefend)
    local tcp_cnt = tcpAttackDefend:getLength()
    local tcp_opt = {[1] = "syn-frag", [2] = "zero-flag", [3] = "syn-fin", [4] = "only-fin"}
    for i = 0, tcp_cnt - 1 do
        table.insert(cmds, string.format("tcp-anomaly %s", tcp_opt[tcpAttackDefend:getItem(i):getValue()]))
    end

    return true
end

local function ddosRulesCliX(obj, ctx)
    local cmds = {}
    local cnt = obj:getLength()
    for i = 0, cnt - 1 do
        local item = obj:getItem(i)
        table.insert(cmds, "config")
        local ruleType = "lan-wan"
        if item:getField("ruleType") ~= nil then
            if item:getField("ruleType"):getValue() == 1 then
                ruleType = "lan-wan"
            elseif item:getField("ruleType"):getValue() == 2 then
                ruleType = "wan-lan"
            end
        end

        table.insert(cmds, string.format("ddos-defense name %s direction %s", item:getField("name"):getValue(), ruleType))
        if item:getField("description") ~= nil and item:getField("description"):getValue() ~= "" then
            table.insert(cmds, string.format("description %s", item:getField("description"):getValue()))
        end

        local src = item:getField("src")
        if src ~= nil then
            local ipGroup = src:getField("ipGroup")
            if ipGroup ~= nil then
                local ipGroup_cnt = ipGroup:getLength()
                for i = 0, ipGroup_cnt - 1 do
                    local ipGroup_item = ipGroup:getItem(i):getRef()
                    table.insert(cmds, string.format("ipgroup %s", ipGroup_item:getField("name"):getValue()))
                end
            end
            
            local zone = src:getField("zone")
            if zone ~= nil then
                local zone_cnt = zone:getLength()
                for i = 0, zone_cnt - 1 do
                    local zone_item = zone:getItem(i):getRef()
                    table.insert(cmds, string.format("src-zone %s", zone_item:getField("name"):getValue()))
                end
            end
        end

        local action = item:getField("action")
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

        ddosRulesCliX_scanFlood(cmds, item:getField("scanDefend"):getField("portScan"), "port-scan", {4000, 300})
        ddosRulesCliX_scanFlood(cmds, item:getField("scanDefend"):getField("ipScan"), "ip-sweep", {4000, 300})

        if ruleType == "lan-wan" and item:getField("lan") ~= nil then
            local ddosDefend = item:getField("lan"):getField("ddosDefend")
            ddosRulesCliX_lanFlood(cmds, ddosDefend:getField("syncFlood"), "syn-flood", {2000, 300})
            ddosRulesCliX_lanFlood(cmds, ddosDefend:getField("udpFlood"), "udp-flood", {2000, 300})
            ddosRulesCliX_lanFlood(cmds, ddosDefend:getField("dnsFlood"), "dns-query-flood", {2000, 300})
            ddosRulesCliX_lanFlood(cmds, ddosDefend:getField("icmpFlood"), "icmp-flood", {2000, 300})
            ddosRulesCliX_lanFlood(cmds, ddosDefend:getField("ackFlood"), "ack-flood", {2000, 300})

            ddosRulesCliX_packetAttackDefend(cmds, item:getField("lan"):getField("attackDefend"):getField("packetAttackDefend"))
        elseif ruleType == "wan-lan" and item:getField("wan") ~= nil then
            local arpDefend = item:getField("wan"):getField("arpDefend")
            if arpDefend:getField("ifThresh") ~= nil then
                if arpDefend:getField("ifThresh"):getValue() ~= 5000 then
                    table.insert(cmds, string.format("arp-flood threshold %d", arpDefend:getField("ifThresh"):getValue()))
                end
            end
        
            if arpDefend:getField("enable") ~= nil then
                if arpDefend:getField("enable"):getValue() ~= true then
                    table.insert(cmds, "no arp-flood")
                end
            end

            ddosRulesCliX_synFlood(cmds, item:getField("wan"):getField("ddosDefend"):getField("syncFlood"), "syn-flood", {2000, 300, 10000, 5000})
            ddosRulesCliX_wanFlood(cmds, item:getField("wan"):getField("ddosDefend"):getField("udpFlood"), "udp-flood", {2000, 300, 100000})
            ddosRulesCliX_wanFlood(cmds, item:getField("wan"):getField("ddosDefend"):getField("icmpFlood"), "icmp-flood", {2000, 300, 4000})
            ddosRulesCliX_wanFlood(cmds, item:getField("wan"):getField("ddosDefend"):getField("dnsFlood"), "dns-query-flood", {2000, 300, 10000})

            ddosRulesCliX_packetAttackDefend(cmds, item:getField("wan"):getField("attackDefend"):getField("packetAttackDefend"))
            ddosRulesCliX_ipv4OptAttackDefend(cmds, item:getField("wan"):getField("attackDefend"):getField("ipv4OptAttackDefend"))
            ddosRulesCliX_tcpAttackDefend(cmds, item:getField("wan"):getField("attackDefend"):getField("tcpAttackDefend"))
        end

        if item:getField("enable"):getValue() ~= true then
            table.insert(cmds, "disable")
        end

        table.insert(cmds, "end\n")
    end

    return table.concat(cmds, "\n")
end

CfgCenter.regConfigBrowser("CLI", "net.ddosRules", browsingGids, ddosRulesCliX)