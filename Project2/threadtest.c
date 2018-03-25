/*
 * Austin Johnson
 * CPSC 3220
 * threadtest.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mythreads.h"
#include "list.h"

ucontext_t maincontext;

#define NUM_THREADS 2

void *threadTest (void *arg) {

    int param = *((int*)arg);
    printf("Test started with %d\n",param);

    threadYield();

	//start critical section
    threadLock(1);
    threadSignal(1,2);
    int* result = malloc(sizeof(int));
    *result = param + 1;
    printf ("added 1! (%d)\n",*result);
    fprintf(stderr, "Waiting...\n");
    threadWait(1,2);
    fprintf(stderr, "Waited on signal...\n");
    *result+=1;
    fprintf(stderr, "added 1! (%d)\n",*result);
    threadUnlock(1);
	//end critical section

    threadYield();

    printf("Test: done result=%d\n",*result);

    return result;
}

int main (){

    //initialize structures / variables
    threadInit();

	int id[NUM_THREADS] = {0};
	int param[NUM_THREADS];
	int i;
	for (i = 0; i < NUM_THREADS; i++) {
		param[i] = rand() % 30;
	}
	int *result[NUM_THREADS] = {NULL};

	//create threads
	for (i = 0; i < NUM_THREADS; i++) {
		fprintf(stderr, "Creating thread %d...\n", i+1);
		id[i] = threadCreate(threadTest, (void *) &param[i]);
	}

	//join threads
	for (i = 0; i < NUM_THREADS; i++) {
		fprintf(stderr, "Joining thread %d...\n", i+1);
		threadJoin(id[i], (void*)&result[i]);
	}

	//exit
	fprintf(stderr, "Exiting...\n");
	threadExit((void*)&result[NUM_THREADS]);

	return 0;
}
