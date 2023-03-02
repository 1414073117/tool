#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include<time.h>

#define REGION_X             100000
#define REGION_Y             100000

typedef unsigned long long int    u64;
typedef unsigned int              u32;
typedef unsigned short            u16;
typedef unsigned char             u8;

struct atomic_state {
    u64 coordinate_x;
    u64 coordinate_y;
    u64 speed;
    u8 direction;
}; 

u64 parclose_x = 50000;
u64 pass[4] = {4000, 40100, 59900, 60000};
u64 light_spot[REGION_Y + 1] = {0};
u64 blocked = 0, hyperline = 0; 

void atomic_state_printf(struct atomic_state *p)
{
    printf("coordinate_x   %llu,", p->coordinate_x);
    printf("coordinate_y   %llu,", p->coordinate_y);
    printf("speed    %llu,", p->speed);
    printf("direction  %u\n", p->direction);
}

u8 passageway(struct atomic_state *p)
{
    if (p->coordinate_x == parclose_x) {
        if (p->coordinate_y >= pass[0] && p->coordinate_y < pass[1]) {
            return 1;
        }
        if (p->coordinate_y >= pass[2] && p->coordinate_y < pass[3]) {
            return 1;
        }

        atomic_state_printf(p);
        blocked++;
        return 0;
    } else if (p->coordinate_x == REGION_X ) {
        light_spot[p->coordinate_y]++;
        atomic_state_printf(p);
        return 0;
    }

    if (p->coordinate_x >= REGION_X || p->coordinate_x < 0) {
        hyperline++;
        atomic_state_printf(p);
        return 0;
    }

    if (p->coordinate_y >= REGION_X || p->coordinate_y <= 0) {
        hyperline++;
        atomic_state_printf(p);
        return 0;
    }

    return 1;
}

u8 forward(struct atomic_state *p, u32 depth)
{
    u8 route;
    depth++;

    if (passageway(p) == 0 || depth >= 100000000) {
        return 0;
    } else {
        route = rand()%10;
        if (route >= 0 && route <= 7) {
            p->direction = 1;
            p->coordinate_x += p->speed;
        } else if (route == 8) {
            p->direction = 2;
            p->coordinate_y += p->speed;
        } else if (route == 9) {
            p->direction = 4;
            p->coordinate_y -= p->speed;
        }
    }

    return forward(p, depth);
}

void launch(u64 amount)
{
    struct atomic_state p = {.speed = 1};
    for (int i = 0; i < amount; i++) {
        p.coordinate_x = 0;
        p.coordinate_y = rand() % (REGION_Y + 1);
        p.direction = 1;
        forward(&p, 0);
    }
}

void sf_type_length(void)
{
    printf("u64:%u\n", sizeof(u64));
    printf("u32:%u\n", sizeof(u32));
    printf("u16:%u\n", sizeof(u16));
    printf("u8 : %u\n", sizeof(u8));
}

void position_printf(void)
{
    printf("REGION_Y   %llu\n", REGION_Y);
    printf("REGION_X   %llu\n", REGION_X);
    printf("blocked    %llu\n", blocked);
    printf("hyperline  %llu\n", hyperline);
    for (int i = 0; i < (REGION_Y + 1); i++) {
        printf("%d  %llu\n", i, light_spot[i]);
    }
}

int main(int argc, char **argv)
{
    srand((unsigned)time(NULL));
    launch(10000);
    position_printf();
    return 0;
}