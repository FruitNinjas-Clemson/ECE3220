/*
*  Austin Johnson
*  CPSC 3220 
*  Project 1
*  leakcount.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
	
	if (argc < 2) {
		fprintf(stderr, "ERROR: Not enough input arguments\n");
		exit(-1);
	}

	pid_t curr_pid = fork();
	if (curr_pid < 0) perror("fork");
	if (curr_pid == 0) {
		//use shim
		char *envp[] = {"LD_PRELOAD=./memory_shim.so", NULL};
		execve(argv[1], argv, envp); 
	}
	else {
		wait(NULL); 
	}  
	return 0;
}
