/*
 * =====================================================================================
 *
 *       Filename:  sleeper.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  01/05/2014 03:45:59 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zi Yan (ZY), zi.yan@gmx.com
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    unsigned i;
    unsigned sleep_time = 10;

    if (argc > 1)
    {
        sleep_time = atoi(argv[1]);
    }

    for (i = 0; i < sleep_time; ++i)
    {
        sleep(1);
        printf("sleep:%d\n", i);
    }

    return 0;
}