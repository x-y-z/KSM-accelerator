#Goal
1. Use LD\_PRELOAD
2. For single-threaded program, *taskset* it to CPU 0(others also be fine)
3. Provide an option to change ksmd CPU affinity
4. Read VIRT, RES of running program from *ps*
5. Read system free memory from *vmstat*
6. Read pages shared, sharing, unshared, volatile from /sys/kernel/mm/ksm/\*
7. Dump stats above periodically, with some parameter to control the frequency