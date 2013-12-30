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
#include <malloc.h>
#include <errno.h>

#include <sys/mman.h>

#define PAGE_4KB (4*1024UL - 8)
#define PAGE_LIST_SZ 4*1024*1024UL

int main()
{
    unsigned long long i;
    unsigned long long j;
    unsigned long long failed = 0, success = 0;
    unsigned long long failed_hp = 0, success_hp = 0;
    void **page_list;
    page_list = (void **)malloc(sizeof(void*)*PAGE_LIST_SZ);
    for (i = 0; i < PAGE_LIST_SZ; ++i)
    {
        /*page_list[i] = malloc(PAGE_4KB);*/
        page_list[i] = valloc(PAGE_4KB);
        if (page_list[i] == NULL)
        {
            printf("page %lld failed", i);
            exit(-1);
        }
        memset(page_list[i], 0, PAGE_4KB);
        int ret = madvise(page_list[i], PAGE_4KB, MADV_MERGEABLE);
        if (ret != 0)
        {
            printf("madvise for mergable failed: %lld, succeeded %lld\n", failed, success);
            ++failed;
            exit(-1);
        }
        else
        {
            ++success;
        }
        /*ret = madvise(page_list[i], PAGE_4KB, MADV_NOHUGEPAGE);*/
        /*if (ret != 0)*/
        /*{*/
            /*++failed_hp;*/
        /*}*/
        /*else*/
        /*{*/
            /*++success_hp;*/
        /*}*/
        /*if (i % 1024 == 0)*/
        /*{*/
            /*printf("%lld\n",i);*/
            /*sleep(10);*/
        /*}*/
    }

    printf("madvise for mergable failed: %lld, succeeded %lld\n", failed, success);
    printf("madvise for huge page failed: %lld, succeeded %lld\n", failed_hp, success_hp);
    /*for (j = 0; j < 900000000000000UL; j++)*/
    /*{*/
    /*}*/
    char a = getchar();

    for (i = 0; i < PAGE_LIST_SZ; ++i)
    {
        free(page_list[i]);
    }

    free(page_list);

    return 0;
}