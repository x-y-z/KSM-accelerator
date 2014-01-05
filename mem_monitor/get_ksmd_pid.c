/*
 * =====================================================================================
 *
 *       Filename:  get_ksmd_pid.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  01/05/2014 03:29:17 PM
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

unsigned int get_ksmd_pid()
{
    char ksmd_pid[20];
    FILE *result = popen("pgrep ksmd", "r");

    if (result != NULL)
    {
        fgets(ksmd_pid, 20, result);
        return (unsigned)atoi(ksmd_pid);
    }
    else
    {
        return 0xdeadbeef;
    }

    return 0;
}
