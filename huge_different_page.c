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
#include <errno.h>

#include <sys/mman.h>

#define PAGE_4KB 4*1024UL
#define PAGE_LIST_SZ 1024UL*1024UL

int main()
{
    int i;
    unsigned long long j;
    void *page;
    page = valloc(PAGE_4KB*PAGE_LIST_SZ);
    if (page == NULL)
    {
        printf("page %d failed", i);
        exit(-1);
    }
    memset(page, 0, PAGE_4KB*PAGE_LIST_SZ);
    int ret = madvise(page, PAGE_4KB*PAGE_LIST_SZ, MADV_MERGEABLE);
    if (ret != 0)
    {
        printf("%d: %s\n", errno, strerror(errno));
        exit(-1);
    }
    /*for (i = 0; i < PAGE_LIST_SZ; ++i)*/
    /*{*/
        /*madvise(page+PAGE_4KB*i/sizeof(void*), PAGE_4KB, MADV_MERGEABLE);*/
    /*}*/
    printf("waiting...\n");

    char a = getchar();
    getchar();

    printf("Change page contents...");

    int *first_8bytes = page;

    for (i = 0; i < PAGE_LIST_SZ; ++i)
    {
        *(first_8bytes + (i * PAGE_4KB/sizeof(int))) = i;
    }

    printf("Done\n");

    a = getchar();

    free(page);

    return 0;
}