/*
 * =====================================================================================
 *
 *       Filename:  map_scanner.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  01/28/2014 03:05:58 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zi Yan (ZY), zi.yan@gmx.com
 *   Organization:
 *
 * =====================================================================================
 */
#include <iostream>
#include <cstdlib>
#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <climits>
#include <getopt.h>
#include <unistd.h>
#include <stdarg.h>

#include <vector>
#include <map>
#include <algorithm>


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using std::vector;
using std::map;
using std::pair;

/*
 * pagemap kernel ABI bits
 */

#define PM_ENTRY_BYTES      sizeof(uint64_t)
#define PM_STATUS_BITS      3
#define PM_STATUS_OFFSET    (64 - PM_STATUS_BITS)
#define PM_STATUS_MASK      (((1LL << PM_STATUS_BITS) - 1) << PM_STATUS_OFFSET)
#define PM_STATUS(nr)       (((nr) << PM_STATUS_OFFSET) & PM_STATUS_MASK)
#define PM_PSHIFT_BITS      6
#define PM_PSHIFT_OFFSET    (PM_STATUS_OFFSET - PM_PSHIFT_BITS)
#define PM_PSHIFT_MASK      (((1LL << PM_PSHIFT_BITS) - 1) << PM_PSHIFT_OFFSET)
#define PM_PSHIFT(x)        (((u64) (x) << PM_PSHIFT_OFFSET) & PM_PSHIFT_MASK)
#define PM_PFRAME_MASK      ((1LL << PM_PSHIFT_OFFSET) - 1)
#define PM_PFRAME(x)        ((x) & PM_PFRAME_MASK)

#define PM_PRESENT          PM_STATUS(4LL)
#define PM_SWAP             PM_STATUS(2LL)


/*
 * kernel page flags
 */

#define KPF_BYTES		8
#define PROC_KPAGEFLAGS		"/proc/kpageflags"

/* copied from kpageflags_read() */
#define KPF_LOCKED		0
#define KPF_ERROR		1
#define KPF_REFERENCED		2
#define KPF_UPTODATE		3
#define KPF_DIRTY		4
#define KPF_LRU			5
#define KPF_ACTIVE		6
#define KPF_SLAB		7
#define KPF_WRITEBACK		8
#define KPF_RECLAIM		9
#define KPF_BUDDY		10

/* [11-20] new additions in 2.6.31 */
#define KPF_MMAP		11
#define KPF_ANON		12
#define KPF_SWAPCACHE		13
#define KPF_SWAPBACKED		14
#define KPF_COMPOUND_HEAD	15
#define KPF_COMPOUND_TAIL	16
#define KPF_HUGE		17
#define KPF_UNEVICTABLE		18
#define KPF_HWPOISON		19
#define KPF_NOPAGE		20
#define KPF_KSM			21

/* [32-] kernel hacking assistances */
#define KPF_RESERVED		32
#define KPF_MLOCKED		33
#define KPF_MAPPEDTODISK	34
#define KPF_PRIVATE		35
#define KPF_PRIVATE_2		36
#define KPF_OWNER_PRIVATE	37
#define KPF_ARCH		38
#define KPF_UNCACHED		39

/* [48-] take some arbitrary free slots for expanding overloaded flags
 * not part of kernel API
 */
#define KPF_READAHEAD		48
#define KPF_SLOB_FREE		49
#define KPF_SLUB_FROZEN		50
#define KPF_SLUB_DEBUG		51

#define KPF_ALL_BITS		((uint64_t)~0ULL)
#define KPF_HACKERS_BITS	(0xffffULL << 32)
#define KPF_OVERLOADED_BITS	(0xffffULL << 48)
#define BIT(name)		(1ULL << KPF_##name)
#define BITS_COMPOUND		(BIT(COMPOUND_HEAD) | BIT(COMPOUND_TAIL))

static const char *page_flag_names[] = {
	[KPF_LOCKED]		= "L:locked",
	[KPF_ERROR]		= "E:error",
	[KPF_REFERENCED]	= "R:referenced",
	[KPF_UPTODATE]		= "U:uptodate",
	[KPF_DIRTY]		= "D:dirty",
	[KPF_LRU]		= "l:lru",
	[KPF_ACTIVE]		= "A:active",
	[KPF_SLAB]		= "S:slab",
	[KPF_WRITEBACK]		= "W:writeback",
	[KPF_RECLAIM]		= "I:reclaim",
	[KPF_BUDDY]		= "B:buddy",

	[KPF_MMAP]		= "M:mmap",
	[KPF_ANON]		= "a:anonymous",
	[KPF_SWAPCACHE]		= "s:swapcache",
	[KPF_SWAPBACKED]	= "b:swapbacked",
	[KPF_COMPOUND_HEAD]	= "H:compound_head",
	[KPF_COMPOUND_TAIL]	= "T:compound_tail",
	[KPF_HUGE]		= "G:huge",
	[KPF_UNEVICTABLE]	= "u:unevictable",
	[KPF_HWPOISON]		= "X:hwpoison",
	[KPF_NOPAGE]		= "n:nopage",
	[KPF_KSM]		= "x:ksm",

	[KPF_RESERVED]		= "r:reserved",
	[KPF_MLOCKED]		= "m:mlocked",
	[KPF_MAPPEDTODISK]	= "d:mappedtodisk",
	[KPF_PRIVATE]		= "P:private",
	[KPF_PRIVATE_2]		= "p:private_2",
	[KPF_OWNER_PRIVATE]	= "O:owner_private",
	[KPF_ARCH]		= "h:arch",
	[KPF_UNCACHED]		= "c:uncached",

	[KPF_READAHEAD]		= "I:readahead",
	[KPF_SLOB_FREE]		= "P:slob_free",
	[KPF_SLUB_FROZEN]	= "A:slub_frozen",
	[KPF_SLUB_DEBUG]	= "E:slub_debug",
};


#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define min_t(type, x, y) ({			\
	type __min1 = (x);			\
	type __min2 = (y);			\
	__min1 < __min2 ? __min1 : __min2; })

#define max_t(type, x, y) ({			\
	type __max1 = (x);			\
	type __max2 = (y);			\
	__max1 > __max2 ? __max1 : __max2; })



struct vma{
    unsigned long start_vpn;
    unsigned long end_vpn;
    vector<unsigned long> pfn_list;
};

static int page_size;
static int kpagecount_fd;
static int kpageflags_fd;
map<pid_t, FILE *> maps_fd;
map<pid_t, int> pagemap_fd;
vector<pid_t> pid_list;
map<pid_t, vector<struct vma> > proc_mapping;
map<unsigned long, unsigned long> pfn_map_counter;


void usage(void)
{
}

vector<pid_t> get_all_pids(char *input)
{
    vector<pid_t> result;
    char *pch;

    pch = strtok(input, " ,");

    while (pch != NULL)
    {
        pid_t id = strtol(pch, NULL, 10);
        pch = strtok(NULL, " ,");
        if (id != 0)
        {
            result.push_back(id);
        }
    }

    return result;
}

FILE *checked_fopen(const char *pathname, const char * flags)
{
	FILE *fd = fopen(pathname, flags);

	if (fd < 0) {
		perror(pathname);
		exit(EXIT_FAILURE);
	}

	return fd;
}

int checked_open(const char *pathname, int flags)
{
	int fd = open(pathname, flags);

	if (fd < 0) {
		perror(pathname);
		exit(EXIT_FAILURE);
	}

	return fd;
}


void open_maps(pid_t id)
{
    char buf[256];
    FILE *file;
    int fd;
    std::cout<<id<<std::endl;

	sprintf(buf, "/proc/%d/pagemap", id);
    fd = checked_open(buf, O_RDONLY);
    pagemap_fd[id] = fd;


	sprintf(buf, "/proc/%d/maps", id);
    file = checked_fopen(buf, "r");
    maps_fd[id] = file;
}
static void fatal(const char *x, ...)
{
	va_list ap;

	va_start(ap, x);
	vfprintf(stderr, x, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}


/*
 * pagemap/kpageflags routines
 */

static unsigned long do_u64_read(int fd, const char *name,
				 uint64_t *buf,
				 unsigned long index,
				 unsigned long count)
{
	long bytes;

	if (index > ULONG_MAX / 8)
		fatal("index overflow: %lu\n", index);

	if (lseek(fd, index * 8, SEEK_SET) < 0) {
		perror(name);
		exit(EXIT_FAILURE);
	}

	bytes = read(fd, buf, count * 8);
	if (bytes < 0) {
		perror(name);
		exit(EXIT_FAILURE);
	}
	if (bytes % 8)
		fatal("partial read: %lu bytes\n", bytes);

	return bytes / 8;
}



static unsigned long pagemap_read(pid_t id, uint64_t *buf,
				  unsigned long index,
				  unsigned long pages)
{
	return do_u64_read(pagemap_fd.at(id),
            "/proc/pid/pagemap",
            buf,
            index,
            pages);
}

static unsigned long pagemap_pfn(uint64_t val)
{
	unsigned long pfn;

	if (val & PM_PRESENT && !(val & PM_SWAP))
		pfn = PM_PFRAME(val);
	else
		pfn = 0;

	return pfn;
}

#define PAGEMAP_BATCH	(64 << 10)
vector<unsigned long> walk_vma(pid_t id, unsigned long index, unsigned long end)
{
	uint64_t buf[PAGEMAP_BATCH];
	unsigned long batch;
	unsigned long pages;
	unsigned long pfn;
	unsigned long i;
    unsigned long count = end - index;
    vector<unsigned long> pfn_list;

	while (count) {
		batch = min_t(unsigned long, count, PAGEMAP_BATCH);
		pages = pagemap_read(id, buf, index, batch);
		if (pages == 0)
			break;

		for (i = 0; i < pages; i++) {
			pfn = pagemap_pfn(buf[i]);
            //push into vma
            pfn_list.push_back(pfn);
		}

		index += pages;
		count -= pages;
	}

    return pfn_list;
}



void get_maps(pid_t id)
{
    char buf[1000];

    if (pagemap_fd.find(id) != pagemap_fd.end() &&
        maps_fd.find(id) != maps_fd.end())
    {
        while (fgets(buf, sizeof(buf), maps_fd.at(id)) != NULL) {
            unsigned long vm_start;
            unsigned long vm_end;
            unsigned long long pgoff;
            int major, minor;
            char r, w, x, s;
            unsigned long ino;
            int n;

            n = sscanf(buf, "%lx-%lx %c%c%c%c %llx %x:%x %lu",
                   &vm_start,
                   &vm_end,
                   &r, &w, &x, &s,
                   &pgoff,
                   &major, &minor,
                   &ino);
            if (n < 10) {
                fprintf(stderr, "unexpected line: %s\n", buf);
                continue;
            }
            struct vma aVMA;
            aVMA.start_vpn = vm_start / page_size;
            aVMA.end_vpn = vm_end / page_size;
            proc_mapping[id].push_back(aVMA);
        }

        for (vector<struct vma>::iterator it = proc_mapping.at(id).begin();
             it != proc_mapping.at(id).end(); ++it)
        {
            it->pfn_list = walk_vma(id, it->start_vpn, it->end_vpn);
        }

    }
}

template<typename T1, typename T2>
void map_print(pair<T1, T2> input)
{
    std::cout<<"(0x"<<std::hex<<input.first<<", "
             <<std::dec<<input.second<<")"<<std::endl;
}

void myclose(pair<pid_t, int> input)
{
    close(input.second);
}


void myfclose(pair<pid_t, FILE*> input)
{
    fclose(input.second);
}

char *page_flag_longname(uint64_t flags)
{
	static char buf[1024];
	int i, n;

	for (i = 0, n = 0; i < ARRAY_SIZE(page_flag_names); i++) {
		if (!page_flag_names[i])
			continue;
		if ((flags >> i) & 1)
			n += snprintf(buf + n, sizeof(buf) - n, "%s,",
					page_flag_names[i] + 2);
	}
	if (n)
		n--;
	buf[n] = '\0';

	return buf;
}


void update_pfn_count(pair<const unsigned long, unsigned long> &input)
{
    uint64_t buf;
    if (kpagecount_fd)
    {
        do_u64_read(kpagecount_fd, "pagecount", &buf, input.first, 1);
        if (input.second > buf)
        {
            std::cerr<<"PFN:0x"<<std::hex<<input.first<<" has count:"<<input.second
                     <<" mappings, but in pagecount:"<<buf;
            if (kpageflags_fd)
            {
                do_u64_read(kpageflags_fd, "pageflags", &buf, input.first, 1);
                std::cerr<<" flags are:"<<page_flag_longname(buf)<<std::endl;
            }
            else
            {
                std::cerr<<std::endl;
            }

        }
        else
        {
            input.second = buf;
        }
    }
}

int main(int argc, char *argv[])
{
	int c;
    const struct option opts[] = {
	{ "pids"       , 1, NULL, 'p' },
	{ "help"      , 0, NULL, 'h' },
	{ NULL        , 0, NULL, 0 }
    };

	page_size = getpagesize();

	while ((c = getopt_long(argc, argv,
				"p:P:h", opts, NULL)) != -1) {
		switch (c) {
		case 'p':
            pid_list = get_all_pids(optarg);
			break;
		case 'h':
			usage();
			exit(0);
		default:
			usage();
			exit(1);
		}
	}

    if (pid_list.empty())
    {
        fprintf(stderr, "Two pids should be specified!\n");
        exit(0);
    }


    for_each(pid_list.begin(), pid_list.end(), open_maps);
    std::cout<<"Open pagemap, maps!"<<std::endl;

    for_each(pid_list.begin(), pid_list.end(), get_maps);
    std::cout<<"Get VPN to PFN mapping!"<<std::endl;

    for (map<pid_t, vector<struct vma> >::iterator it = proc_mapping.begin();
         it != proc_mapping.end(); ++it)
    {
        std::cout<<"Process: "<<it->first<<" mappings are below:"<<std::endl;
        for (vector<struct vma>::iterator it2 = it->second.begin();
             it2 != it->second.end(); ++it2)
        {
            for (unsigned long vpn = it2->start_vpn;
                 vpn != it2->end_vpn; ++vpn)
            {
                try{
                    unsigned long pfn = it2->pfn_list.at(vpn-it2->start_vpn);
                    if (pfn)
                    {
                    //std::cout<<"VPN: 0x"<<std::hex<<vpn<<" -> "
                             //<<"PPN: 0x"<<pfn
                             //<<std::endl;
                        pfn_map_counter[pfn]++;
                    }
                }
                catch(std::exception e)
                {
                }
            }
        }

    }

    kpagecount_fd = checked_open("/proc/kpagecount", O_RDONLY);
    kpageflags_fd = checked_open("/proc/kpageflags", O_RDONLY);

    for_each(pfn_map_counter.begin(), pfn_map_counter.end(), update_pfn_count);

    //for_each(pfn_map_counter.begin(), pfn_map_counter.end(), map_print<unsigned long, unsigned long>);

    close(kpageflags_fd);
    close(kpagecount_fd);

    for_each(pagemap_fd.begin(), pagemap_fd.end(), myclose);
    for_each(maps_fd.begin(), maps_fd.end(), myfclose);
    return 0;
}