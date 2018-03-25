/*
*  Austin Johnson
*  CPSC 3220 
*  Project 1
*  sctracer.c
*/

#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define NUM_SYS_CALLS 326

//structure to keep up with occurrences
//for each system call made
typedef struct sys_call_occurrence_tag {
	int sys_call_num;
	int occurrences;
} sys_call_occurrence;

//function prototype
void write_traced_syscalls_to_file (sys_call_occurrence arr[], char *filename);

int main(int argc, char **argv) {

   if (argc < 2) {
	fprintf(stderr, "ERROR: Missing arguments\n");
	exit(-1);
   }

    //system call array
    sys_call_occurrence system_calls[NUM_SYS_CALLS];
    int i;
    for (i = 0; i < NUM_SYS_CALLS; i++) {
	system_calls[i].sys_call_num = i;
	system_calls[i].occurrences = 0;
    }

    pid_t child = fork();
    if (child < 0) perror("fork");
    if (child == 0) {

	//get ready to be traced
        ptrace(PTRACE_TRACEME);

        //allow the parent to trace
        kill(getpid(), SIGSTOP);

        //trace another program passed in from the command line
	if (strchr(argv[1], ' ') == NULL) execvp(argv[1], argv);	
	else {		

		//count number of arguments in argv[1]
		int i, num_arguments;
		char *token1, *copy = argv[1];		
		for (num_arguments = 1; *copy; copy++)
			if (*copy == ' ') num_arguments += 1;
		char *tokens[num_arguments+1];	

		//get tokens
		for (i = 0; i < num_arguments; i++) {
			if (i == 0) {
				token1 = strtok(argv[1], " ");
				tokens[i] = token1;
			}
			else tokens[i] = strtok(NULL, " ");
		}
		tokens[num_arguments] = NULL;	
		execvp(token1, tokens);		
	}

    } else {
        int status,syscall_num;

        //wait for the child to stop itself
        waitpid(child, &status, 0);

        //this option makes it easier to distinguish normal traps from
        //system calls
        ptrace(PTRACE_SETOPTIONS, child, 0,
                PTRACE_O_TRACESYSGOOD);

	while(1) {
		do{

		    //wait for a system call
		    ptrace(PTRACE_SYSCALL, child, 0, 0);

		    //actually wait for child status to change
		    waitpid(child, &status, 0);

		    // if the child exited...let's exit too
		    if (WIFEXITED(status)) { 
			write_traced_syscalls_to_file(system_calls, argv[argc-1]);
			exit(1);
		    }
		      
		    //read out the saved value of the RAX register,
		    //which contains the system call number
	  	    //For 32-bit machines, you would use EAX.
	 	    syscall_num = ptrace(PTRACE_PEEKUSER, child, sizeof(long)*ORIG_RAX, NULL);

		    //increment system call occurence
		    system_calls[syscall_num].occurrences += 1;

		//wait until the process is stopped or bit 7 is set in
		//the status (see man page comment on
		//PTRACE_O_TRACESYSGOOD)
		} while (!(WIFSTOPPED(status) && WSTOPSIG(status) & 0x80));

		ptrace(PTRACE_SYSCALL, child, 0, 0);
		waitpid(child, &status, 0);	
	}
    }

    return 0;
}

/*
   This function writes the system calls that were made in ascending order,
   along with how many times each call occurred
*/
void write_traced_syscalls_to_file ( sys_call_occurrence arr[], char *filename) {
	
	//use bubble sort to sort the array in 
	//descending order based off num of occurrences	
	int i, j;
	sys_call_occurrence temp;
	for (i = 0; i < NUM_SYS_CALLS - 1; i++) {
		for (j = 0; j < NUM_SYS_CALLS - 1 - i; j++) {
			if (arr[j].occurrences < arr[j+1].occurrences) {
				temp = arr[j+1];
				arr[j+1] = arr[j];
				arr[j] = temp;
			}
		}
	}	
	//count number of system calls that occur
	int last_index_with_occurrence = 0;
	while (arr[last_index_with_occurrence + 1].occurrences > 0) {
		last_index_with_occurrence++;
	}
	//use bubble sort to sort the array in 
	//ascending order based off sys call num
	for (i = 0; i < last_index_with_occurrence; i++) {
		for (j = 0; j < last_index_with_occurrence - 1 - i; j++) {
			if (arr[j].sys_call_num > arr[j+1].sys_call_num) {
				temp = arr[j+1];
				arr[j+1] = arr[j];
				arr[j] = temp;
			}
		}
	}
	//write sys calls with occurrences to file
	FILE *fp = fopen(filename, "w");
	if (fp == NULL) {
		fprintf(stderr, "ERROR: File unable to open\n");
		exit(-1);
	}
	for (i = 0; i <= last_index_with_occurrence; i++) 
		fprintf(fp, "\n%d\t%d", arr[i].sys_call_num, arr[i].occurrences);
	fclose(fp);
}



