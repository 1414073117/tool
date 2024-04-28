#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SFVPN_IN_GET_ROUTE_TUNNEL_CRC SFVPN_IN_GET_ROUTE_TUNNEL_CRC

// sfvpn回调注册类型枚举
typedef enum {
    SFVPN_ENCRYPT_SESSION_INFO_TYPE,    // sfvpn加密流程向外吐出会话相关信息
    SFVPN_DECRYPT_SESSION_INFO_TYPE,    // sfvpn解密流程向外吐出会话相关信息
    SFVPN_IN_GET_ROUTE_TUNNEL_CRC,      // 对加密前数据包查询目的sfvpn隧道crc
    SFVPN_OUT_GET_ROUTE_TUNNEL_CRC,     // 对解密后数据包查询源sfvpn隧道crc
    SFVPN_REG_TYPE_MAX,
} sfvpn_reg_type_t;



int main()
{
    #ifdef SFVPN_IN_GET_ROUTE_TUNNEL_CRC
    printf("001\n");
    #endif
    return 0;
}
