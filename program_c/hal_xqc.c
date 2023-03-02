#include <stdio.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <ctype.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include "sfidl/idl_object.h"
#include "sfidl/idl_file_mgr.h"
#include "sfidl/idl_file.h"
#include "sfidl/idl_service.h"
#include "sfidl/idl_error.h"
#include "sfidl/idl_impl.h"
#include "sfidl/idl_engine.h"
#include "sfidl/idl_cutils.h"

#include <base/oserror.h>


typedef unsigned long long int    u64;
typedef unsigned int              u32;
typedef unsigned short            u16;
typedef unsigned char             u8;		

static idl_client_st* s_hal_client = NULL;

static int dp_led_trigger(const char *name, int rgb, u32 mode)
{
	idl_object_st *ret;
    idl_client_st *client = s_hal_client;

	/* led-name + mode + rgb*/
    idl_object_st *args[] = {
        idl_object_new_string((const char *)name),
        idl_object_new(schema_mgr_get(idl_file_get_schema_mgr(IDL_MF(client->mgr)), "hal.led.mode")),
        idl_object_new_int32(rgb),
    };

	//设置mode
    int rval = idl_object_set_value(args[1], &mode);
    if (rval) {
        goto clean_arg;
    }

	//async 调用
    rval = idl_client_sync_call(client->eng, "HalLedSet", args, sizeof(args)/sizeof(void *), 1, &ret);
    if (rval) {
		return -1;
	}

    int _bool;
	idl_object_get_bool_value(ret, &_bool);
    printf("return:%d\n", _bool);

clean_arg:
    idl_object_release(args[0]);
    idl_object_release(args[1]);
    idl_object_release(args[2]);
	return 0;
}

static void dp_led_get_list(void)
{
	idl_object_st *ret;
    idl_client_st *client = s_hal_client;

	/* 获取现有传感器 */
	int rval = idl_client_sync_call(client->eng, "HalLedList", NULL, 0, 1, &ret);
	if (rval) {
		return;
	}
	
	/* 迭代ret */
	int sc = idl_object_get_length(ret);
	int i;
	for (i = 0; i < sc; ++i) {
		idl_object_st *it;
		rval = idl_object_get_item(ret, i, &it);
		if (rval) {
			continue;
		}
		const char *str;
		u32 slen;
		rval = idl_object_get_string_value(it, &str, &slen);
		if (rval) {
			continue;
		}
        printf("led %s\n", str);
	}

	idl_object_release(ret);
}


void dp_led_ctrl_cb(int type)
{
#define SF_HAL_LED_OPT_SYNC		0xF1 //操作阻塞直到硬件开启
#define SF_HAL_LED_OPT			0xF1 //开启
#define SF_HAL_LED_TWINKLE		0XF2 //闪烁
//rgb表示颜色的三维，这里1是开启，0是关闭
#define SF_HAL_LED_RGB_OPEN		1
#define SF_HAL_LED_RGB_CLOSE	0
//该name从hal代码中拿到的
#define SF_HAL_HA_LED_NAME		"-ha-led-plane-sangfor"

    if (type == 1) {
        dp_led_trigger(SF_HAL_HA_LED_NAME, SF_HAL_LED_RGB_OPEN, SF_HAL_LED_OPT);
    } else if(type == 2) {
        dp_led_trigger(SF_HAL_HA_LED_NAME, SF_HAL_LED_RGB_OPEN, SF_HAL_LED_TWINKLE);
    } else {
        dp_led_trigger(SF_HAL_HA_LED_NAME, SF_HAL_LED_RGB_CLOSE, SF_HAL_LED_OPT);
    }
}


int dp_hal_client_init(void)
{
    char file_path[128] = "/sfos/system/schema/service/hal.schema";

#define FAKE_MD5 "a62ef4c7885061061baaea9db49738d2"

    s_hal_client = idl_cutils_client_init((const char *)file_path, FAKE_MD5, "driver.service", "rpc", 3);
    if (!s_hal_client) {
        return -1;
    }


    return 0;
}

int dp_hal_init(void)
{
    if (dp_hal_client_init() < 0) {
        return -1;
    }

    return 0;
}

void dp_hal_fini(void)
{
    if (s_hal_client) {
		idl_cutils_client_exit(s_hal_client);
		s_hal_client = NULL;
    }
}


void* ptintf_file_lock(void* tid)
{
    int wh = *((int*)tid);
    int sp = 0;

    while (wh) {
        wh --;
        sp = rand()%3;
        printf("type %d\n", sp);
        dp_led_ctrl_cb(sp);
    }

    return NULL;
}

void multiple_concurrency(int be, void* wh)
{
    pthread_t td[be];
    srand((unsigned)getpid());
    dp_hal_init();
    for (int i = 1; i < be; i++) {
        if (pthread_create(&td[i], NULL, ptintf_file_lock, wh) != 0) {
            printf("pthread_create error\n");
            assert(0);
        }
    }

    ptintf_file_lock(wh);

    for (int i = 1; i < be; i++) {
        pthread_join(td[i], NULL);
    }

    dp_hal_fini();
    return;
}

void fork_multiple_concurrency(int fk ,int be, void* wh)
{
    pid_t fpid;
    for (int i = 1; i < fk; i++) {
        fpid = fork();
        if (fpid== 0) {
            multiple_concurrency(be, wh);
            return;
        }
    }

    multiple_concurrency(be, wh);
    return;
}

int main(int argc, char **argv)
{
    int whsize, be, fk;
    if (argc < 2) {
        printf("hhhhhhhhhhhhhhhhhh");
        return 0;
    }

    if (!strcmp(argv[1], "1")) {
        dp_hal_init();
        dp_led_get_list();
        dp_hal_fini();
    } else if (!strcmp(argv[1], "2")) {
        whsize = atoi(argv[2]);
        be = atoi(argv[3]);
        fk = atoi(argv[4]);
        fork_multiple_concurrency(fk, be, (void*)&whsize);
    } else if (!strcmp(argv[1], "3")) {
        dp_hal_init();
        whsize = atoi(argv[2]);
        dp_led_ctrl_cb(whsize);
        dp_hal_fini();
    }

    return 0;
}
