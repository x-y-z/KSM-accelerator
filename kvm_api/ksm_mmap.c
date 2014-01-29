// =====================================================================================
//
//       Filename:  ksm_mm.c
//
//    Description:
//
//        Version:  1.0
//        Created:  12/29/2013 02:44:36 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  YOUR NAME (),
//        Company:
//
// =====================================================================================

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

/*#define _GNU_SOURCE*/
#include <dlfcn.h>

static void* (*real_mmap)(void*, size_t, int, int, int, off_t)=NULL;

static void mtrace_init(void)
{
    real_mmap = dlsym(RTLD_NEXT, "mmap");
    if (NULL == real_mmap) {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
}

void *mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    if(real_mmap == NULL) {
        mtrace_init();
    }

    void *p = NULL;
    /*fprintf(stderr, "malloc(%ld) = ", size);*/
    p = real_mmap(addr, length, prot, flags, fd, offset);
    if (p != NULL && ((flags & MAP_ANONYMOUS) != 0))
    {
        madvise(p, length, MADV_MERGEABLE);
    }
    fprintf(stderr, "mmap: %p\n", p);
    return p;
}
/*

void *calloc(size_t num, size_t size)
{
    if(real_malloc==NULL) {
        mtrace_init();
    }

    void *p = NULL;
    p = real_malloc(num*size);
    if (p != NULL)
    {
        memset(p, 0, num*size);
        madvise(p, num*size, MADV_MERGEABLE);
    }
    return p;
}

*/
/*
void *realloc(void *ptr, size_t size)
{
    if(real_malloc==NULL) {
        mtrace_init();
    }

    void *p = NULL;
    p = real_malloc(size);
    if (p != NULL)
    {
        madvise(p, size, MADV_MERGEABLE);

        if (ptr != NULL)
        {
            memcpy(p, ptr, size);
            free(ptr);
        }
    }

    return p;
}
*/
