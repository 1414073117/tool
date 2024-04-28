import sfidl
import os

os_root = os.getenv('OSROOT') or '/sfos/system'
hal_schema_file = os_root + '/etc/schema/service/hal.schema'
hal_lib_name = "libos_wrapper.so"
hal_driver_name = "driver.service"

def get_device_id():
    """
    获取设备网关序列号
    """

    srv = sfidl.loadSchema(hal_schema_file)
    lib = srv.getServiceImpls(hal_lib_name)
    print(lib.HalGetDeviceID())

def get_device_list():
    """
    获取内存设备列表
    """

    srv = sfidl.loadSchema(hal_schema_file)
    lib = srv.getServiceImpls(hal_lib_name)
    print(lib.HalMemoryDeviceList())

def get_sengsor_list():
    """
    获取当前传感器列表
    """

    srv = sfidl.loadSchema(hal_schema_file)
    driver = srv.getServiceEngine(hal_driver_name, "rpc", False, 20)
    print(driver.HalSensorList())

def get_sengsor_all_read():
    """
    获取全部传感器信息
    """

    srv = sfidl.loadSchema(hal_schema_file)
    driver = srv.getServiceEngine(hal_driver_name, "rpc", False, 20)
    sensorList = driver.HalSensorList()

    for sensor in sensorList:
        print(sensor,driver.HalSensorRead(sensor))

if __name__ == "__main__":
    get_device_id()
    get_device_list()
    get_sengsor_list()
    get_sengsor_all_read()