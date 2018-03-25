Austin Johnson
CPSC 3220
Program1


KNOWN PROBLEMS: No known bugs from the tests that I have ran against it. 

DESCRIPTION: 
    1. Program #1: Memory Leak Detector - This program uses a shim library to 
   	inspect the calls to malloc and free. It accepts a command line argument
	that turns the specified command into a separate process. That process 
	then goes through the shim library and after the process executes, the
	memory leaks (if any) are printed out. 
    
    2. Program #2: System Call Tracer - This program uses the ptrace API to count
	how many times a system call occurs. It accepts a command line argument that 
	turns the child process into the executable. It keeps up with the number 
	of occurences that each system call happens within that process and writes the 
	results into an output file (specified by the user through the command line). Each
	system call number that occured is printed along with how many times it occured. 
	The results are ordered in order of increasing system call number. 

DESIGN:
    1. Program #1: Memory Leak Detector - The memory leak detector I implemented uses a 
	singly linked list to keep track of all of the blocks of memory that were allocated. 
	When the user calls free, the block is freed and removed from the list. At the end
	of the program when the destructor is called, the list is printed along with the total
	number of leaks and bytes leaked. If no leaks are detected, the total number of leaks 
	will be 0 along with 0 bytes leaked. 
    
    2. Program #2: System Call Tracer - The system call tracer I implemented used the ptrace API
	to count all system calls from an executable passed by the user. It is similiar to the 
	implementation on github -> processes folder -> ptrace_example.c. It allows the child 
	process (executable) to be traced with ptrace through the parent. It continues to trace 
	until the process is stopped or the status changes. I am using an array of size 326 (the 
	number of system calls). Whenever a system call occurs, the index corresponding to that 
	call (sys_call #100 equals index 100) is incremented by one. Once the process finishes, 
	the array is sorted in increasing order of sys_call numbers. The sys_call numbers that 
	have occurrences are then printed, along with how many times they were called. 

