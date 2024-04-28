#!/usr/bin/env lloader

SCRIPTPATH = os.getenv("SFVPN_ROOT_PATH") or "/sf/sgp/sfvpn"
package.path = SCRIPTPATH.."/lualibs/?.lua;"..os.getenv("LUA_PATH")
package.cpath = SCRIPTPATH.."/lualibs/?.so;"..os.getenv("LUA_CPATH")

local os_vpn_pbr = require "os_vpn_pbr"

local function add_vpn_pbr_route(family, ipaddr, gateway, panel)
    local ret, msg = os_vpn_pbr.addVpnPbrRoute(family, ipaddr, gateway, panel)
    if ret == false then
        print("add error:", msg)
    end
end


local function del_vpn_pbr_route(family, ipaddr, gateway, panel)
    local ret, msg = os_vpn_pbr.delVpnPbrRoute(family, ipaddr, gateway, panel)
    if ret == false then
        print("del error:", msg)
    end
end

local function cs_i1()
    for i = 1, 254, 1 do
        add_vpn_pbr_route("inet", "192.168.1."..tostring(i), "192.168.1.254", "eth1")
        add_vpn_pbr_route("inet6", "2001:1::"..tostring(i), "2001:1::fffe", "eth1")
    end
end

local function cs_d1()
    for i = 1, 254, 1 do
        del_vpn_pbr_route("inet", "192.168.1."..tostring(i), "192.168.1.254", "eth1")
        del_vpn_pbr_route("inet6", "2001:1::"..tostring(i), "2001:1::fffe", "eth1")
    end
end

local function yz()
    load xqc_1
    load xqc_2 = nil
    load xqc_3 = ""
    load xqc_4 = "xqc"
    if xqc_1 then
        print("1 yes")
    end
    if xqc_2 then
        print("2 yes")
    end
    if xqc_3 then
        print("3 yes")
    end
    if xqc_4 then
        print("4 yes")
    end
end

function main()
    cs_i1()
end

main()