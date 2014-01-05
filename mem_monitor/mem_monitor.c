// =====================================================================================
//
//       Filename:  mem_monitor.c
//
//    Description:
//
//        Version:  1.0
//        Created:  12/30/2013 02:45:18 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  YOUR NAME (),
//        Company:
//
// =====================================================================================
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "stats_reader.h"

struct global_args{
    int program_cpu_affinity;    /* -1: any cpu, 0-N: cpu id */
    int ksm_cpu_affinity;        /* -1: any cpu, 0-N: cpu id */
    unsigned int dump_frequency; /* it is in second as a parameter of sleep */
    char **program_args;         /* running program and its arguments  */
};

struct global_args parse_cmd_arguments(int argc, char **argv);
void printUsage();

int main(int argc, char **argv)
{

    pid_t running_program = -1;
    pid_t parent_program = getpid();

    struct global_args g_args;
    unsigned int i;

    char *env[] = {"LD_PRELOAD=../kvm_api/ksm_mm.so", NULL};

    g_args = parse_cmd_arguments(argc, argv);

    if (g_args.program_args == NULL)
    {
        printUsage();
        exit(0);
    }

    for (i = 0; g_args.program_args[i] != NULL; ++i)
    {
        printf("%s\n", g_args.program_args[i]);
    }

    running_program = fork();

    if (running_program == 0) // child
    {
        printf("I am child, and my pid is: %d\n", getpid());
        execve(g_args.program_args[0], g_args.program_args,
               env);
        //TODO:Get LD_PRELOAD in
        /*sleep(10);*/
        /*printf("child is dying\n");*/
    }
    else if (running_program < 0) // failed to fork
    {
        fprintf(stderr, "Failed to fork!\n");
        exit(1);
    }
    else //parent
    {
        printf("I am parent, and my pid is: %d, my child's pid is %d\n",
                getpid(), running_program);

        int child_status = 0;
        while (!waitpid(running_program, &child_status, WNOHANG))
        {
            printf("waiting...\n");
            //TODO: Read vmstat, VIRT, RES, ksm stats
            unsigned long virt, rss;
            struct ksm_stats one_stats;

            get_virt_rss_kb(running_program, &virt, &rss);
            one_stats = get_ksm_stats();
            printf("Free mem:%ld, pid %d: VIRT %ldKB, RSS %ldKB\n",
                   get_free_memory_kb(), running_program, virt, rss);
            printf("KSM:\n full_scans:%ld\n pages_shared:%ld\n pages_sharing:%ld\n "
                   "pages_unshared:%ld\n pages_volatile:%ld\n",
                   one_stats.full_scans, one_stats.pages_shared,
                   one_stats.pages_sharing, one_stats.pages_unshared,
                   one_stats.pages_volatile);

            sleep(g_args.dump_frequency);

        }
    }
    return 0;
}


void printUsage()
{
    printf("Usage:\n"
           "    --program_cpu\t \n"
           "    --ksm_cpu\t \n"
           "    --sampling_rate\t \n"
           "    --program\t \n");
}

struct global_args parse_cmd_arguments(int argc, char **argv)
{
    static struct option long_options[] =
    {
        {"program_cpu", required_argument, 0, 'p'},
        {"ksm_cpu", required_argument, 0, 'k'},
        {"sampling_rate", required_argument, 0, 's'},
        {"program", required_argument, 0, 'r'}
    };
    struct global_args res_args = {0, 1, 1, NULL};

    int opt_idx = 0;
    int c;
    int input;
    char *pch;
    unsigned i, str_len, param_len = 0;
    while ((c = getopt_long_only(argc, argv, "p:k:s:r:",
                                 long_options, &opt_idx)) != -1)
    {
        switch(c)
        {
            case 0:
                if (long_options[opt_idx].flag != 0)
                    break;
                printUsage();
                break;
            case 'p':
                res_args.program_cpu_affinity = atoi(optarg);
                break;
            case 'k':
                res_args.ksm_cpu_affinity = atoi(optarg);
                break;
            case 's':
                input = atoi(optarg);
                if (input > 0)
                {
                    res_args.dump_frequency = (unsigned int)input;
                }
                break;
            case 'r':
                str_len = strlen(optarg);
                for (i = 0; i < str_len; ++i)
                {
                    if (optarg[i] == ' ')
                    {
                        ++param_len;
                    }
                }
                res_args.program_args = (char **)malloc(
                                        sizeof(char*)*(param_len+2));
                i = 0;

                pch = strtok(optarg, " ");
                while (pch != NULL)
                {
                    res_args.program_args[i++] = pch;
                    /*printf("%s\n", pch);*/
                    pch = strtok(NULL, " ");
                }
                res_args.program_args[i] = NULL;
                break;
            default:
                printUsage();
                break;
        }
    }

    return res_args;
}