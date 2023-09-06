#include "a3.h"

void toSignal(int s){
    // retrieve input
    signal(s, SIG_IGN);
    printf("Are you sure you want to quit? Y/N ");
    char input = getchar();

    //handle the signal
    if (input == 'Y' || input == 'y') exit(0);
    else signal(SIGINT, toSignal);
    getchar();
}

// intercept the signal at ctrl+z
void intercept(int s){}

// struct for storing cpu
typedef struct{
    char cpuVal[MAX];
    double current;
    double time;
    double utilization;
} cpuStruct;

// struct for storing memory
typedef struct{
    char memory[MAX];
    double previous;
} memoryStruct;

int main(int argc, char *argv[]){

    // setting variables to know which commands will be called
    int samp = 10, td = 1;
    bool sys = false, user = false, graph = false, seq = false, sampCheck = false, tdCheck = false;

    //run signals
    signal(SIGTSTP, intercept);
    signal(SIGINT, toSignal);

    for (int i = 1; i<argc; i++){

        // initializing variables connected to each command using flags
        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--system") == 0 ) sys = true;

        else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--user") == 0) user = true;
            
        else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--graphics") == 0) graph = true;
            
        else if (strcmp(argv[i], "-seq") == 0 || strcmp(argv[i], "--sequential") == 0) seq = true;
            
        else if (strncmp(argv[i], "-sa=", 4) == 0 || strncmp(argv[i], "--samples=", 10) == 0){
            //check that a user has provided valid input for number of samples
            sampCheck = true;
            samp = check_validity(argv[i], samp, 3, 9);
            
        }else if (strncmp(argv[i], "-td=", 4) == 0 || strncmp(argv[i], "--tdelay=", 9) == 0){
            //check that a user has provided valid input for number of samples
            tdCheck = true;
            td = check_validity(argv[i], td, 3, 8);

        }else{
            // if a user has not provided a command line argument, check if 
            // input is a valid value for a positional argument
            if (atoi(argv[i]) != 0){

                // based on what other commands have been called, determine whether the
                // value inputted should represent samples or tdelay
                
                if (!tdCheck && !sampCheck || tdCheck && !sampCheck){
                    sampCheck = true;
                    samp = check_validity(argv[i], samp, 0, 0);
                }else if (!tdCheck && sampCheck){
                    tdCheck = true;
                    td = check_validity(argv[i], td, 0, 0);
                }
            }
        }
    } 

    //initializing variables to be reused throughout the iterations 
    char memArr[samp][MAX], cpuArr[samp][MAX], memStr[MAX], userStr[MAX], cpuStr[MAX];
    memoryStruct memory; 
    cpuStruct cpu;

    //iterating through the samples
    for (int i = 0; i < samp; i++){

        // FORKS AND PIPES

        // initialize strings
        strcpy(memStr, "");
        strcpy(cpuStr, "");
        strcpy(userStr, "");

        //check for errors
        int memPipe[2], userPipe[2], cpuPipe[2];
        if (pipe(memPipe) != 0 || pipe(userPipe) != 0 || pipe(cpuPipe) != 0) {
            fprintf(stderr, "Pipe creation failure");
            exit(EXIT_FAILURE);}

        int status;
        pid_t memPID, userPID, cpuPID;

        // read and write data information with forks and pipes 
         for (int j = 0; j < 3; j++) {

           // working with memory
           if (j == 0) {
                
               if ((memPID = fork()) == -1) fprintf(stderr, "Fork failure");

               else if (memPID == 0) {

                   close(memPipe[0]);
                   getting_memory(memory.memory, i, graph, &memory.previous);

                    // check for errors
                    if ((status = write(memPipe[1], &memory, sizeof(memory))) < 0) {
                        fprintf(stderr, "Pipe writing error");
                        exit(EXIT_FAILURE);
                    }

                   close(memPipe[1]); }

               else{
                   
                   while(wait(NULL) > 0);
                   close(memPipe[1]);

                   // check for errors
                   if ((status = read(memPipe[0], &memory, sizeof(memory))) < 0) {
                        fprintf(stderr, "Pipe writing error");
                        exit(EXIT_FAILURE);}

                   // copy this value to the memory array
                   strcpy(memArr[i], memory.memory);
                   close(memPipe[0]);}

            // working with user
           }else if (j == 1) {
               
               if ((userPID = fork()) == -1) fprintf(stderr, "Fork failure");

               else if (userPID == 0) {

                   close(userPipe[0]);
                   getting_user(userStr);

                   // check for errors
                    if ((status = write(userPipe[1], userStr, strlen(userStr) + 1)) < 0) {
                        fprintf(stderr, "Pipe writing error");
                        exit(EXIT_FAILURE);}

                   close(userPipe[1]);}

               else {
                    while(wait(NULL) > 0);
                    close(userPipe[1]);

                    // check for errors
                   if ((status = read(userPipe[0], userStr, sizeof(userStr))) < 0) {
                        fprintf(stderr, "Pipe writing error");
                        exit(EXIT_FAILURE);}

                    close(userPipe[0]);}

           // working with cpu
           }else if (j == 2) {

               if ((cpuPID = fork()) == -1) fprintf(stderr, "Fork failure");

               else if (cpuPID == 0) {
                   close(cpuPipe[0]);
                   getting_cpu(cpu.cpuVal, i, graph, &cpu.current, &cpu.time, &cpu.utilization);

                    // check for errors
                    if ((status = write(cpuPipe[1], &cpu, sizeof(cpu))) < 0) {
                        fprintf(stderr, "Pipe writing error");
                        exit(EXIT_FAILURE);}}

               else {
                   while(wait(NULL) > 0);
                   close(cpuPipe[1]);

                    // check for errors
                   if ((status = read(cpuPipe[0], &cpu, sizeof(cpu))) < 0) {
                        fprintf(stderr, "Pipe writing error");
                        exit(EXIT_FAILURE);}

                   // copy this value to the cpu array
                   strcpy(cpuArr[i], cpu.cpuVal);
                   close(cpuPipe[0]);}
           }
       }
        
       // exiting our children processes
        
        if (memPID == 0) exit(EXIT_SUCCESS);
        if (userPID == 0) exit(EXIT_SUCCESS);
        if (cpuPID == 0) exit(EXIT_SUCCESS);

        // MAKING FUNCTION CALLS

        // if sequential is not called, clear between each sample
        if (!seq){
            printf("\033[H \033[2J \n");
            printf("Nbr of samples: %d -- every %d secs\n", samp, td);
        }
        else printf("\n>>> iteration %d\n", i+1);
        
        // use the u_info value of a rusage struct to retireve total memory usage
        getting_usage();

        // if -user has been called but not -system, print only user information
        if (user && !sys){
            printf("--------------------------------\n");
            printf(" ### Sessions/users ###");
            printf("%s\n", userStr);
            printf("--------------------------------\n");
            
        // if -system has been called but not -user, print only system information
        }else if (sys && !user){
            printing_memory(memArr, i, seq, samp);
            printf("--------------------------------\n");
            printing_cpu(cpuArr, i, graph, seq, samp, cpu.current);

        // if both -user and -system have been called or if neither have been called,
        // print all information
        }else{
            printing_memory(memArr, i, seq, samp);
            printf("--------------------------------\n");
            printf(" ### Sessions/users ###");
            printf("%s\n", userStr);
            printf("--------------------------------\n");
            printing_cpu(cpuArr, i, graph, seq, samp, cpu.current);
        }
        // delay program by tdelay # of seconds between each sample
        sleep(td);
    }
    // always print out system information
    printing_sysinfo();
    return 0;
}

