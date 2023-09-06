# SysProg - Systems Monitoring and FD Tables Tool

## Description
This is a program intended to display information about a computer system. The information available includes memory usage, CPU usage, current user sessions, number of cores, number of samples and delay time.
I used concurrency by forking 3 times within a for loop. At each loop, I examined the different processes - first memory, then users, then cpu usage. Within the forks, I don’t correspond with the parent until waiting for the child to finish. When the for loop has completed, that’s when the children are exited - this maintains concurrency. The pids of the children are all kept track of, and exiting them allows us to ensure no zombies remain. We can confirm this by running htop and seeing that there is no indication of zombie processes.
I used SIGINT to intercept the Ctrl+C signal, so my code now asks the user to confirm whether they’d like to exit the program. I used SIGTSTP to intercept the Ctrl+Z signal, and this functionality does not do anything.

## Libraries

The libraries required for installation are listed in the assigment3.h header file as such:
```
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
```

We also define a constant to use throughout our program:
```
#define MAX 1024
```

## Functions

**stats_functions.c**

```
check_validity(char argv2[], int x, int s1, int s2)
```

This function takes a char array ```argv2```, and it checks whether this is a valid argument. It checks if the character at indicated s1 or s2 are an = sign, meaning this is an argument intended to represent a tdelay value or a sample value. If so, this function checks whether the remainder of the argument is a valid value, and it will update x with that value and return x. If there is no = sign, we take this to be a positional argument, and we again check whether the number is a valid integer. If not, we return x as its original value. This function has no return type.

```
printing_sysinfo()
```
This function accesses the ``` sys/utsname.h``` library to collect information from a utsname struct. It uses the ```uname()``` function to retrieve all the information regarding the information of the system type, including its name, machine name, version, release, and architecture. This function has no parameters and no return type, but will print all the information to standard output.

```
getting_user()
```
This function retrieves information from the ```utmp.h``` library. It uses a utmp pointer alongside the ```setutent()```, ```getutent()```, and ```endutent()``` functions. These functions of the utmp library allow us to access the file and a pointer within to retrieve our information and output the personal information connected to each user. We run a while loop in this function to iterate through all the existing users, then terminate this function when all users have been displayed. This function has no parameters and no return type.
```
getting_usage()
```
 
 This function uses the ```sys/resource.h``` file to access a rusage struct and receive information about the total memory usage. The data inside this rusage struct is displayed for our total memory usage. This function has no parameters and no return type.

```
getting_cpu(char cpuArr[MAX], int i, bool graph, double *cpu, double *time, double *utilization)
```
This function uses the ```/proc/stat``` file to retrieve information. Reading the first line of the file, we add the seven values to receive our total time, noting that the fourth value represents idle time. Between separate calls of this function, we have our program sleep for tdelay # of seconds, then repeat this process, storing values in pointers that we can calculate the difference between. We divide this value by the total difference and multiply by 100 to receive a total percentage. We have an array arr that stores the final CPU value.

```
printing_cpu(char cpuArr[MAX], int i, bool graph, bool seq, int samp, double cpu)
```

This function will print the number of cores as well as the total cpu usage, and based on whether the graphics or sequential flag is used, the display will reflect this.

```
void getting_memory(char memArr[MAX], int i, bool graph, double *previous)
```
This function uses ```sys/sysinfo.h``` to collect information from the sysinfo struct, namely collecting the current and total usage values of both physical and virtual memory. It takes an array and will update the current memory usage statistics as a string. We have array memArr that stores the current virtual memory value of the system by concatenating it to the existing string.
If graphics is not selected as a command, the program simply displays the current memory usage. If graphics is selected, we look at our pointers where we have the previous value stored, and we compare the current iteration value from the previous to get a difference. With that value, based on whether it’s positive or negative, we respectively print that quantity of # symbols or : symbols, followed by a * to represent positive change or a @ to represent a decrease. This function has no return type.

```
printing_memory(char cpuArr[MAX], int i, bool seq, int samp)
```

 This function will print the used and total memory, and based on whether the sequential flag is used, the display will reflect this.
 
**assignment3.c**

```
void toSignal(int s)
```

This function retrieves input from the user, and if the user inputs ‘y’ or ‘Y’, this signals to the code that the program should quit. This function is an interception of the Ctr+C function.
```

void intercept(int s)
```

This function is an interception of the Ctrl+Z function, and it has no body - there is no functionality, meaning the code will continue running.

```
typedef struct node {} cpuStruct
```

This initializes a new struct to store the value of the cpu string, current cpu value, time
taken, and total utilization for a process.
```
typedef struct node {} memoryStruct
```

This initializes a new struct to store the value of the memory string as well as the
previous memory value.
```
int main(int argc, char *argv[])
```
In this function, we set boolean variables that represent each of the flags so we know when a flag has been selected. The first for loop parses through all the inputted arguments, and it will set booleans to true accordingly. For tdelay and samples, the booleans are set to true and the initialized variables for these values are updated.

We then use pipes and forks to concurrently have these processes running. We error check throughout the code to see if there are any errors with forking or piping. We run a for loop three times, forking once for each type of usage we want to look at - first memory, then users, then cpu. We store values in the arrays and strings that we initialized before this loop so we can print them out for each iteration. After we’ve gone through the for loop, we make sure to exit all the children processes.

Inside the for-loop iterating through all the samples, we clear or do not clear the screen based on whether or not sequential has been called. Based on whether the boolean values are true for users but not system, system but not users, both, or neither, we will call on our previously defined functions to display the respective information. Within each of these functions, whether the graphics flag has been selected will be used to determine what is displayed by each function.

At the end of each iteration of this loop, we print getting_sysinfo as we are always retrieving that information. We then sleep the program for the number of seconds indicated by the tdelay value.


## USAGE
**Flags:**
```
--system can also be written as -s
--user can also be typed as -u
--graphics can also be typed as -g
--sequential can be written as -seq
--samples=N can also be written as -sa=N or be taken as a single or the first of multiple positional arguments.
--tdelay=T can also be written as -td=T or be taken as the second of two or more positional arguments.
```

NOTE: If samples has already been defined by the user by a samples flag, and there is one or more positional arguments provided afterwards, the first positional argument will represent tdelay e.g. ```-sa=3 5``` will have the first positional argument, 5, taken as the tdelay value since samples is already defined.
The values for tdelay and sample must be integers greater than 0 to be valid in this program.

 The user may input any combination of the flags for them to be valid.
This program will ignore any invalid commands and implement valid ones. If only invalid commands are entered, it will print all the information, as if no commands were entered.
For graphics and calculating the difference between iterations, I subtracted the current iteration from the past iteration and multiplied by 10, so the graphics displayed on the screen show the difference multiplied by 10 as often the difference is extremely miniscule, too small to even be recorded.
This program can be run by running make, or by doing gcc asignment3.c stats_functions.c where both files are in the same directory.
