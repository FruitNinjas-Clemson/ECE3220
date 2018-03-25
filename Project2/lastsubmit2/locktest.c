#include <stdio.h>
#include <stdlib.h>
#include "mythreads.h"

int x = 0;

void* t2 (void *arg) {
    printf("Got the value %d\n",*((int*)arg));
    return NULL;
}

void *t1 (void *arg)
{
    int param = *((int*)arg);
    printf("t1 started %d\n",param);

    // Simple yield
    threadYield();

    // Simple Lock
    threadLock(1);

    // This signal is important for the second thread...
    threadSignal(1,2);

    int* result = malloc(sizeof(int));
    *result = param + 1;
    printf ("added 1! (%d)\n",*result);

    // Simple Wait
    threadWait(1,2);

    // End of critical section
    threadUnlock(1);

    // Signal the second thread
    threadSignal(1,2);

    // Simple yield
    threadYield();

    printf("t1: done result=%d\n",*result);

    // Thread from a thread
    if(x++ == 0) {
        int p3 = 45;
        threadCreate(t2,(void*)&p3);
    }

    return result;
}

int main(void)
{
    int id1, id2, id3;
    int p1;
    int p2;
    int p3;

    p1 = 23;
    p2 = 2;
    p3 = 5;

    int *result1, *result2, *result3;

    //initialize the threading library. DON'T call this more than once!!!
    threadInit();

    id1 = threadCreate(t1,(void*)&p1);
    printf("created thread 1.\n");

    id2 = threadCreate(t1,(void*)&p2);
    printf("created thread 2.\n");

    printf("Let's try and join!\n");
    threadJoin(id1, (void*)&result1);
    printf("joined #1 --> %d.\n",*result1);

    threadJoin(id2, (void*)&result2);
    printf("joined #2 --> %d.\n",*result2);

    return 0;
}
