#include "a3.h"


// function to check if an argument is valid for our program
int check_validity(char argv2[], int x, int s1, int s2){
    int index = 0;
    bool valid = true;
    char val[1024];

    // find index of = to see where value begins
    if (argv2[s1] == '=') index = s1+1;
    else if (argv2[s2] == '=') index = s2+1;
    
    // check that input is valid (only numeric)
    strcpy(val, &argv2[index]);
    if (strlen(argv2) != 1){
        for (int v = index; v<strlen(argv2); v++){
            if ((!(argv2[v] <= '9' && argv2[v] >= '0'))){ 
                valid = false;
                break;
            }
        }
    }
    // update value if valid
    if (atoi(val)!=0 && valid) x = atoi(val);
    return x;
}


// function to receive system information
void printing_sysinfo(){

    // use a utsname struct and its records to collect 
    // desired information about the system
    struct utsname info;
    uname(&info);
    //printf("--------------------------------\n");
    printf("### System Information ###\n");
    printf(" System Name: %s\n", info.sysname);
    printf(" Machine Name: %s\n", info.nodename);
    printf(" Version: %s\n", info.version);
    printf(" Release: %s\n", info.release);
    printf(" Architecture: %s\n", info.machine);
    printf("--------------------------------\n");
}


// function to retrieve all the user and display their information
void getting_user(char user[MAX]){

    // use a utmp pointer alongside the utent() functions
    struct utmp *the_user;
    setutent();
    while (the_user = getutent()){

        // print out each user's information
        if (the_user->ut_type == USER_PROCESS){
            sprintf(user + strlen(user), "\n%s         %s  %s", the_user->ut_user, the_user->ut_line, the_user->ut_host);

        }
    }
    endutent();
}


// function to retrieve total memory usage
void getting_usage(){
    struct rusage u_info;
    getrusage(RUSAGE_SELF, &u_info);
    printf("Memory usage: %ld kilobytes\n", u_info.ru_maxrss);
}


// function to collect cpu usage
void getting_cpu(char cpuArr[MAX], int i, bool graph, double *cpu, double *time, double *utilization){

    // open the /proc/stat file and read the 7 values into variables
    FILE *core_stats = fopen("/proc/stat", "r");
    int a, b, c, d, e, f, g, current;
    double total, last = 0;
    fscanf(core_stats, "cpu %d %d %d %d %d %d %d", &a, &b, &c, &d, &e, &f, &g);
    
    // add all the values up, noting that the fourth value (d) represents idle time
    total = (double)a+b+c+d+e+f+g;
    current = total-d;

    // close the file
    if (fclose(core_stats) != 0) {
        fprintf(stderr, "fclose failed\n");
        return;
    }

    // compute the differences of total time and idle time to retrieve percentage, store in last
    if (i > 0) last = ( (current - *utilization) / (total - *time) )*100;

    // if the user selects graphics
    if (graph){

        char str[1024];
        int x = abs(last) + 1;

        // use memset to display the respective number of bars
        memset(str, '\0', sizeof(str));
        memset (str,'|', x);
        str[x] = '\0';

        // store new value in array 
        strcpy(cpuArr, "\n         ");
        sprintf(cpuArr + strlen(cpuArr), "%s %.2lf", str, last);
    }
    
    // adjust pointer values
    *utilization = current;
    *time = total;
    *cpu = last;
}


// function for printing cpu usage
void printing_cpu(char cpuArr[][MAX], int i, bool graph,  bool seq, int samp, double cpu){

    printf("Number of cores: %ld", sysconf(_SC_NPROCESSORS_ONLN));
    printf("\n total cpu use = %.2f%%\n", cpu);

    // if the user selects graphics
    if (graph){

        if (!seq){
            
            for (int j = 0; j <=i; j++) printf("%s", cpuArr[j]);
        }
        else{
            for (int j = 0; j<=i; j++) {};
            printf("%s", cpuArr[i]); 
        }
        for (int j = samp; j > i; j--) printf("\n");
    }
    printf("--------------------------------\n");
}


// function to collect memory information
void getting_memory(char memArr[MAX], int i, bool graph, double *previous){

    // use a sysinfo struct to retrieve memory usage statistics
    double total, current, totalUsed, currentUsed;
    struct sysinfo mem;
    sysinfo(&mem);

    // calculate the current and total usage and divide to get correct byte size 
    total =  (double)mem.totalram/1024/1024/1024 + (double)mem.totalswap/1024/1024/1024;
    totalUsed = (double)mem.totalram/1024/1024/1024 - (double)mem.freeram/1024/1024/1024;
    current = (double) (total - ((double)mem.freeram/1024/1024/1024 + (double)mem.freeswap/1024/1024/1024));
    currentUsed = (double)mem.totalram/1024/1024/1024;
    sprintf(memArr, "%.2lf GB / %.2lf GB -- %.2lf GB / %.2lf GB", totalUsed, currentUsed, current, total);

    // if the user selects graphics
    if (graph){
        strcat(memArr, "    |");
        char str[MAX];

        // set memory diffrence and get values to display graphics
        double diff = current - *previous;
        double absDiff = fabs(diff);
        int x = 10 * absDiff;

        // use memset to display the characters indicating positive or negative change
        memset(str, '\0', sizeof(str));

        if (i == 0) strcat(memArr, "-");
        
        else{

        if (diff > 0){
            memset (str,'#',x);
            str[x] = '*';
        }
        else if (diff < 0){
            memset (str,':',x);
            str[x] = '@';
        }
        str[x+1] = '\0';

        if (diff == 0 || (diff > 0 && diff < 0.1)) strcat(memArr, "o");
        else strcat(memArr, str);
        }
        // concatenate to array and update pointer

        int length = strlen(memArr);         
        sprintf(memArr + length, " %.2lf (%.2lf)", absDiff, current);
                printf("%s\n", memArr);

        *previous = current;
    }
}


// function to print memory information
void printing_memory(char memArr[][MAX], int i, bool seq, int samp){

    printf("--------------------------------\n");
    printf(" ### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");

        if (!seq){
            for (int j = 0; j<=i; j++) printf("%s\n", memArr[j]);}

        else{
            for (int j = 0; j<i; j++) printf("\n");
            printf("%s", memArr[i]); }

        for (int j = samp; j > i; j--) printf("\n");
}