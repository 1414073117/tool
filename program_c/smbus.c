#ifdef __KERNEL__
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/pci.h>
#include <asm/io.h>
#include <linux/hardirq.h>
#else
#include <stdio.h>
#include <sys/io.h>
#include <unistd.h>
#endif
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>


#define SMBUS_HSTS_REG (s_smbus_io_base + 0x00)

#define C610_SMBUS_CONTROLLER_DEFAULT_BASE_ADDR 0x0580
u_int16_t s_smbus_io_base = C610_SMBUS_CONTROLLER_DEFAULT_BASE_ADDR;

// #define C3000_SMBUS_CONTROLLER_DEFAULT_BASE_ADDR 0xE000
// u_int16_t s_smbus_io_base = C3000_SMBUS_CONTROLLER_DEFAULT_BASE_ADDR;


int main(int argc, char **argv)
{
    u_int8_t status = inb(SMBUS_HSTS_REG);
    printf("xqc :%u\n", status);
    return 0;
    
}