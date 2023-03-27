local browsingGids = {
    "cfg.nicHangParams",
    "cfg.nicHangCheck",
    "cfg.nicHangCheck.hangCheckSwitch"
}

local function nicHangParams(obj, ctx)
    local cmds = {}
    if obj:getField("params"):getField("hangCheckSwitch"):getValue() == true then
        table.insert(cmds, "config")
        table.insert(cmds, "nic-hang-check enable")
        table.insert(cmds, "end\n")
    end

    return table.concat(cmds, "\n")
end

CfgCenter.regConfigBrowser("CLI", "cfg.nicHangParams", browsingGids, nicHangParams)