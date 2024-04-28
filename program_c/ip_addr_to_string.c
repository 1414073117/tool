#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef unsigned long long int    u64;
typedef unsigned int              u32;
typedef unsigned short            u16;
typedef unsigned char             u8;
typedef unsigned long long int    sf_uint64_t;
typedef unsigned int              sf_uint32_t;
typedef unsigned short            sf_uint16_t;
typedef unsigned char             sf_uint8_t;

#define SF_L3_IPV4_MASK        (0xFFFF0000)
#define SF_L3_IPV4_MARK        (0xFFBF0000)

typedef struct sf_ipaddr6_
{
    union
    {
        u8    addr8[16];
        u16    addr16[8];
        u32    addr32[4];
        u64    addr64[2];
    };
} sf_ipaddr6_t;

typedef union sf_ip_addr_ {
    union {
        struct {
            u32 __pad[3];
            u32 ipv4;
        };
        sf_ipaddr6_t ipv6;
    };

#define addr_ipv4  ipv6.addr32[3]
#define addr_ipv6  ipv6.addr32
} sf_ip_addr_t;

sf_uint8_t sf_ip_address_is_ip4(const sf_ip_addr_t *ip46)
{
    return ((ip46->addr_ipv6[0] & sf_htonl(SF_L3_IPV4_MASK)) == sf_htonl(SF_L3_IPV4_MARK));
}

int sf_ip_address_to_string(const sf_ip_addr_t *ip46, char *buf, sf_uint32_t size)
{
    if (sf_ip_address_is_ip4(ip46)) {
        const unsigned char *ipv4 = (unsigned char *) ip46->ipv4;
        snprintf(buf, size, "%d.%d.%d.%d", ipv4[0], ipv4[1], ipv4[2], ipv4[3]);
    } else {
        const unsigned char *ipv6 = (unsigned char *) ip46->ipv6.addr8;
        snprintf(buf, size, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
            ipv6[0], ipv6[1], ipv6[2], ipv6[3], ipv6[4], ipv6[5], ipv6[6], ipv6[7],
            ipv6[8], ipv6[9], ipv6[10], ipv6[11], ipv6[12], ipv6[13], ipv6[14], ipv6[15]);
    }
}

int main()
{

}