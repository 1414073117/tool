#!/usr/bin/env lloader

local filter = '$[?(@.__isDefault == true)]'
local filter1 = '$[?(@.__isDefault == true)]'
filter = string.format("$[?(@.__isDefault == true)][?(@.vline <> $[?(@.ifname <> $[?(@.ifType == \"%s\")])])]", "xqc0041")
filter1 = filter1..string.format("[?(@.vline <> $[?(@.ifname <> $[?(@.ifType == \"%s\")])])]", "xqc0041")
print(filter)
print(filter1)