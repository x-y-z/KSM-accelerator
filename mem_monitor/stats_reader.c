// =====================================================================================
//
//       Filename:  stats_reader.c
//
//    Description:
//
//        Version:  1.0
//        Created:  12/31/2013 10:20:11 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  YOUR NAME (),
//        Company:
//
// =====================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "stats_reader.h"
#define BUFFER_SZ 256

struct mem_info get_free_memory_kb()
{
    FILE *meminfo;
    int bytes_read;
    char line_buf[BUFFER_SZ];
    struct mem_info ret_mem_info = {1, 1, 1, 1, 1, 1};

    meminfo = fopen("/proc/meminfo", "r");
    while (fgets(line_buf, BUFFER_SZ, meminfo) != NULL)
    {
        if (strstr(line_buf, "MemTotal") != NULL)
        {
            ret_mem_info.total = strtoul(&line_buf[10], NULL, 10);
        }
        else if (strstr(line_buf, "MemFree") != NULL)
        {
            ret_mem_info.free = strtoul(&line_buf[9], NULL, 10);
        }
        else if (strstr(line_buf, "Buffers") != NULL)
        {
            ret_mem_info.buffered = strtoul(&line_buf[9], NULL, 10);
        }
        else if (strstr(line_buf, "SwapCached") != NULL)
        {
            ret_mem_info.swap_cached = strtoul(&line_buf[11], NULL, 10);
        }
        else if (strstr(line_buf, "Cached") != NULL)
        {
            ret_mem_info.cached = strtoul(&line_buf[8], NULL, 10);
        }
        else if (strstr(line_buf, "SwapTotal") != NULL)
        {
            ret_mem_info.swap_total = strtoul(&line_buf[10], NULL, 10);
        }
        else if (strstr(line_buf, "SwapFree") != NULL)
        {
            ret_mem_info.swap_free = strtoul(&line_buf[9], NULL, 10);
        }
    }
    fclose(meminfo);

    return ret_mem_info;
}

void get_virt_rss_kb(pid_t pid, unsigned long *virt, unsigned long *rss)
{
    unsigned long virt_res = 0, rss_res = 0;
    char proc_status_file_name[32];
    FILE *proc_status;
    char line_buf[BUFFER_SZ];

    sprintf(proc_status_file_name, "/proc/%d/status", pid);

    proc_status = fopen(proc_status_file_name, "r");

    while (fgets(line_buf, BUFFER_SZ, proc_status) != NULL)
    {
        if (strstr(line_buf, "VmSize") != NULL)
        {
            virt_res = strtoul(&line_buf[7], NULL, 10);
        }
        else if (strstr(line_buf, "VmRSS") != NULL)
        {
            rss_res = strtoul(&line_buf[6], NULL, 10);
        }
    }
    if (virt != NULL)
    {
        *virt = virt_res;
    }
    if (rss != NULL)
    {
        *rss = rss_res;
    }

    fclose(proc_status);

    return;
}
struct ksm_stats get_ksm_stats()
{
    struct ksm_stats res_stats = {0,0,0,0,0};

#define READ_ITEM(var, field) \
    { \
        FILE *item_file;  \
        char line_buf[BUFFER_SZ]; \
        item_file = fopen("/sys/kernel/mm/ksm/"#field, "r");  \
        if (fgets(line_buf, BUFFER_SZ, item_file) != NULL) \
        { \
            var.field = strtoul(line_buf, NULL, 10); \
        } \
        fclose(item_file); \
    }


    READ_ITEM(res_stats, full_scans);
    READ_ITEM(res_stats, pages_shared);
    READ_ITEM(res_stats, pages_sharing);
    READ_ITEM(res_stats, pages_unshared);
    READ_ITEM(res_stats, pages_volatile);

#undef READ_ITEM


    return res_stats;
}
/*
int main()
{
    int i;
    unsigned long virt, rss;
    struct ksm_stats one_stats;
    for (i = 0; i < 10; ++i)
    {
        get_virt_rss_kb(148, &virt, &rss);
        one_stats = get_ksm_stats();
        printf("Free mem:%ld, pid 2400: VIRT %ldKB, RSS %ldKB\n",
               get_free_memory_kb(), virt, rss);
        printf("KSM:\n full_scans:%ld\n pages_shared:%ld\n pages_sharing:%ld\n "
               "pages_unshared:%ld\n pages_volatile:%ld\n",
               one_stats.full_scans, one_stats.pages_shared,
               one_stats.pages_sharing, one_stats.pages_unshared,
               one_stats.pages_volatile);
        sleep(1);
    }
    return 0;
}
*/