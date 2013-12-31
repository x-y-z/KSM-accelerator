// =====================================================================================
//
//       Filename:  stats_reader.h
//
//    Description:
//
//        Version:  1.0
//        Created:  12/31/2013 10:20:14 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  YOUR NAME (),
//        Company:
//
// =====================================================================================

struct ksm_stats{
    unsigned long pages_shared;
    unsigned long pages_sharing;
    unsigned long pages_unshared;
    unsigned long pages_volatile;
    unsigned long full_scans;
};

unsigned long get_free_memory_kb();
void get_virt_rss_kb(pid_t pid, unsigned long *virt, unsigned long *rss);
struct ksm_stats get_ksm_stats();

