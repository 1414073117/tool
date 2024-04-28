import sfidl
import os
import json
import sys
import traceback


schema_file = "/sfos/system/schema/service/status.center.schema"
tenant = os.getenv("CC_TENANT") #用户名字默认：tena 可以修改成xqc001
db = os.getenv("CC_DB")
ipath = '$'

vsysSchemaFile = "/sfos/system/schema/config/status-center/config.if.status.schema"
vsysSchema = "ifStatusList"


def getjgt(cstenant,csfile,csname):
    schema = sfidl.loadSchema(schema_file)
    cc_serv = schema.getServiceEngine("status.center", "rpc", False, 3)
    objs = cc_serv.Query(csname, ipath)
    logsetting_schema = sfidl.loadSchema(csfile)
    result = logsetting_schema.convert(csname, objs)
    print("{\"",cstenant,"\":\"",cstenant,"\",\"",csname,"\":",json.dumps(result),"}")
    return result


if __name__ == "__main__":
    vsys = getjgt(tenant,vsysSchemaFile,vsysSchema)
    
