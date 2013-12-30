// =====================================================================================
//
//       Filename:  all_zero_pages.c
//
//    Description:
//
//        Version:  1.0
//        Created:  12/21/13 11:29:46
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

#include <sys/mman.h>

#define PAGE_4KB 4*1024
#define PAGE_LIST_SZ 1024*1024

int main()
{
    int i;
    unsigned long long j;
    void **page_list;
    page_list = (void **)malloc(sizeof(void*)*PAGE_LIST_SZ);
    for (i = 0; i < PAGE_LIST_SZ; ++i)
    {
        page_list[i] = malloc(PAGE_4KB);
        if (page_list[i] == NULL)
        {
            printf("page %d failed", i);
            exit(-1);
        }
        memset(page_list[i], 0, PAGE_4KB);
        madvise(page_list[i], PAGE_4KB, MADV_MERGEABLE);
        ((int*)page_list[i])[0] = i % 4;
    }

    /*for (j = 0; j < 900000000000000UL; j++)*/
    /*{*/
    /*}*/
    sleep(30);

    for (i = 0; i < PAGE_LIST_SZ; ++i)
    {
        free(page_list[i]);
    }

    free(page_list);

    return 0;
}