GCC=gcc

mem_monitor: mem_monitor.c stats_reader.c get_ksmd_pid.c
	$(GCC) -o $@ $^
mem_monitor_no_ksm: mem_monitor_no_ksm.c stats_reader.c get_ksmd_pid.c
	$(GCC) -o $@ $^
