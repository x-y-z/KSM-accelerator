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
#include <sys/mman.h>

/*#define _GNU_SOURCE*/
#include <dlfcn.h>

static void* (*real_malloc)(size_t)=NULL;

static void mtrace_init(void)
{
    real_malloc = dlsym(RTLD_NEXT, "valloc");
    if (NULL == real_malloc) {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
}

void *malloc(size_t size)
{
    if(real_malloc==NULL) {
        mtrace_init();
    }

    void *p = NULL;
    /*fprintf(stderr, "malloc(%ld) = ", size);*/
    p = real_malloc(size);
    madvise(p, size, MADV_MERGEABLE);
    /*fprintf(stderr, "%p\n", p);*/
    return p;
}
