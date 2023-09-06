#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <utmp.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <math.h>
#include <sys/sysinfo.h>
#include <getopt.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX 1024

#ifndef __A3
#define __A3

int check_validity(char argv2[], int x, int s1, int s2);
void printing_sysinfo();
void getting_user(char user[MAX]);
void getting_usage();
void getting_cpu(char cpuArr[MAX], int i, bool graph, double *cpu, double *time, double *utilization);
void printing_cpu(char cpuArr[][MAX], int i, bool graph,  bool seq, int samp, double cpu);
void getting_memory(char memArr[MAX], int i, bool graph, double *previous);
void printing_memory(char memArr[][MAX], int i, bool seq, int samp);

#endif