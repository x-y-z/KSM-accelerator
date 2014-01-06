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

#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <iostream>

#include <cstring>

#include <sys/mman.h>

#define PAGE_4KB 4*1024UL
#define PAGE_LIST_SZ 1024UL*1024UL

struct huge_page {
    uint32_t dummy[1024*PAGE_LIST_SZ];
};

int main()
{
    int i;
    unsigned long long j;
    struct huge_page *page;
    page = new struct huge_page;
    if (page == NULL)
    {
        std::cerr<<"memory cannot be allocated"<<std::endl;
        return -1;
    }
    memset(page, 0, PAGE_4KB*PAGE_LIST_SZ);
    std::cout<<"waiting...\n";

    std::cin>>i;

    std::cout<<"Change page contents...";

    int *first_8bytes = reinterpret_cast<int*>(page);

    for (i = 0; i < PAGE_LIST_SZ; ++i)
    {
        *(first_8bytes + (i * PAGE_4KB/sizeof(int))) = i;
    }

    std::cout<<"Done\n";

    std::cin>>i;
    std::cin>>i;

    delete (page);

    return 0;
}