import sfidl
import os
import json
import sys
import traceback


schema_file = os.getenv("CC_SCHEMA")
tenant = os.getenv("CC_TENANT") #用户名字默认：tena 可以修改成xqc001
db = os.getenv("CC_DB")
ipath = '$'

vsysSchemaFile = "/sfos/system/etc/schema/config/networkplatform/config.vsys.schema"
vsysSchema = "cfg.vsys"
vsysSchema = "cfg.vsys"

Schema = {
    # "/sfos/system/etc/schema/config/networkplatform/config.appcontrol.local.schema" : "policy.appcontrol.local.policys",
    # "/sfos/system/etc/schema/config/networkplatform/config.appcontrol.schema" : "policy.appcontrol",
    # "/sfos/system/etc/schema/config/networkplatform/config.dnsdetect.schema" : "cfg.activeDetects",
    # "/sfos/system/etc/schema/config/networkplatform/config.dnsproxy.schema" : "net.dnsproxy",
    # "/sfos/system/etc/schema/config/networkplatform/config.dnstransparentproxy.schema" : "net.dnstransparentproxy",
    # "/sfos/system/etc/schema/config/networkplatform/config.netobj.schema" : "net.netobjs",
    # "/sfos/system/etc/schema/config/networkplatform/config.serv.schema" : "net.servs",
    "/sfos/system/etc/schema/config/networkplatform/config.zone.schema" : "net.zones",
    # "/sfos/system/etc/schema/config/networkplatform/config.interface.schema" : "net.interfaces",
    # "/sfos/system/etc/schema/config/networkplatform/config.ripnginterface.schema" : "net.ripng.if",
    # "/sfos/system/etc/schema/config/networkplatform/config.ripng.schema" : "net.ripngs",
    # "/sfos/system/etc/schema/config/networkplatform/config.ripngnetwork.schema" : "net.ripng.networks",
    # "/sfos/system/etc/schema/config/networkplatform/config.pbr4.schema" : "net.pbr4List",
    # "/sfos/system/etc/schema/config/networkplatform/config.pbr6.schema" : "net.pbr6List",
    # "/sfos/system/schema/config/afplatform/config.appcontrol.conflict.schema" : "policy.appcontrol.conflicts",
    # "/sfos/system/etc/schema/config/networkplatform/config.schedule.schema" : "net.schedules",
    # "/sfos/system/etc/schema/config/afplatform/config.logsetting.schema" : "cfg.logsetting",
    # "/sfos/system/etc/schema/config/networkplatform/config.appcontrol.local.droplist.schema" : "cfg.analylocal",
    # "/sfos/system/etc/schema/config/networkplatform/config.networkparams.schema" : "cfg.networkParams",
    # "/sfos/system/schema/config/serviced/config.contsec.schema" : "cfg.contsectemplate",
}

def getjgt(cstenant,csfile,csname):
    schema = sfidl.loadSchema(schema_file)
    cc_serv = schema.getServiceEngine("cfg.center", "rpc", False, 3)
    fd = cc_serv.Open(cstenant, db, [csname], schema.newObject("cc.mode", "w"), "acl_local")
    objs = cc_serv.Query(fd, csname, ipath)
    cc_serv.Close(fd)
    logsetting_schema = sfidl.loadSchema(csfile)
    result = logsetting_schema.convert(csname, objs)
    print("{\"",cstenant,"\":\"",cstenant,"\",\"",csname,"\":",json.dumps(result),"}")
    return result


if __name__ == "__main__":
    vsys = getjgt(tenant,vsysSchemaFile,vsysSchema)
    vsysname = []
    if vsys["enable"] == True:
        for vsys_id in vsys["list"]:
            if vsys_id["name"] == "public":
                vsysname.append(tenant)
            else:
                vsysname.append(vsys_id["name"])
    else:
        vsysname.append(tenant)
    for name in vsysname:
        for key,values in  Schema.items():
            getjgt(name,key,values)
    
