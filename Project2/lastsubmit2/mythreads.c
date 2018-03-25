/*
 * Austin Johnson
 * CPSC 3220
 * mythreads.c
 *
 */

#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mythreads.h"
#include "list.h"

//global lists of running
//and exited threads
thread_list *thread_pool;
thread_list *completed_threads;

//global list of threads waiting
//on a condition
thread_list *waiting_threads[NUM_LOCKS][CONDITIONS_PER_LOCK];

//boolean array to keep up with
//which locks are taken
int locks[NUM_LOCKS] = {0};

//increment this every time a thread is
//created to give all threads a unique id
int UNIQUE_THREAD_ID = 0;

//boolean variable for atomic operations
int interruptsAreDisabled = 0;

//function prototypes
void threadReturn();
void threadCleanup();
static void interruptDisable();
static void interruptEnable();

/*
    Initializes the thread library. Creates three lists: thread_pool
    for currently running threds, completed_threads for exited threads,
    and waiting_threads for threads waiting on a condition variable. Also
    creates the main context.
*/
void threadInit() {
    //create thread pool
    thread_pool = (thread_list *) malloc(sizeof(thread_list));

    //create completed threads list
    completed_threads = (thread_list *) malloc(sizeof(thread_list));

    //create waiting threads list
    int i, j;
    for (i = 0; i < NUM_LOCKS; i++) {
        for (j = 0; j < CONDITIONS_PER_LOCK; j++) {
            waiting_threads[i][j] = (thread_list *) malloc(sizeof(thread_list));
            waiting_threads[i][j]->num_curr_threads = 0;
        }
    }

    //insert main context
    thread_data *main_thread = malloc(sizeof(thread_data));
    main_thread->ID = UNIQUE_THREAD_ID++;
    main_thread->context = malloc(sizeof(ucontext_t));
    getcontext(main_thread->context);
    main_thread->funcPtr = NULL;
    main_thread->argPtr = NULL;
    main_thread->func_return = 0;
    main_thread->next = NULL;
    list_insert(thread_pool, main_thread);
    thread_pool->num_curr_threads = 1;
    completed_threads->num_curr_threads = 0;
}

/*
    Creates a thread with a stack of size STACK_SIZE and
    executes the function passed to it with funcPtr through
    a wrapper function called threadReturn. This function
    returns the thread's unique ID.
*/
int threadCreate(thFuncPtr funcPtr, void *argPtr) {

    //create a new thread
    thread_data *new_thread = malloc(sizeof(thread_data));
    new_thread->ID = UNIQUE_THREAD_ID++;
    new_thread->context = malloc(sizeof(ucontext_t));
    getcontext(new_thread->context);
    new_thread->funcPtr = funcPtr;
    new_thread->argPtr = argPtr;

    // allocate and initialize a new call stack
    new_thread->context->uc_stack.ss_sp = malloc(STACK_SIZE);
    new_thread->context->uc_stack.ss_size = STACK_SIZE;
    new_thread->context->uc_stack.ss_flags = 0;

    // create the new context --> set the void (*func) argument of
    // makecontext to point to threadReturn in order to store the
    // return value of the funcPtr in the thread's structure
    makecontext (new_thread->context, (void(*)(void)) threadReturn, 0);

    // put the thread onto end of list since it is about to be executed
    thread_data *old_thread = thread_pool->tail;
    list_insert(thread_pool, new_thread);

    // execute the new thread
    swapcontext (old_thread->context, new_thread->context);

    return new_thread->ID;
}

/*
    Causes the currently running thread (the tail of thread_pool)
    to "yield" the processor to the next runnable thread (the head
    of thread_pool)
*/
void threadYield() {
    interruptDisable();
    thread_data *running_thread = thread_pool->tail;
    thread_data *thread_to_run = list_remove(thread_pool, HEAD);
    list_insert(thread_pool, thread_to_run);
    interruptEnable();
    swapcontext(running_thread->context, thread_to_run->context);
}

/*
    Waits for the thread corresponding to parameter thread_id
    to exit. If the return value of the function the thread
    was running in is not NULL, then result is set equal to the
    return value. If the thread has already exited or the thread_id
    does not exits, threadJoin returns immediately.
*/
void threadJoin(int thread_id, void **result) {

    //make sure main context calls this function and the
    //result pointer is valid (allocated for)
    interruptDisable();
    assert(thread_pool->tail->ID == 0);

    //check if thread corresponding to thread_id already
    //exited or does not exist. If so, return from function
    if (thread_id < 0 || thread_id > UNIQUE_THREAD_ID) {
        interruptEnable();
        return;
    }
    thread_data *rover = completed_threads->head;
    while (rover != NULL) {
        if (rover->ID == thread_id) {
            interruptEnable();
            if (result != NULL)
                *result = rover->func_return;
            return;
        }
        rover = rover->next;
    }

    //wait for thread corresponding to thread_id to exit
    while (1) {
        if (completed_threads->num_curr_threads > 0 &&
          completed_threads->tail->ID == thread_id) { //thread exited
            if (result != NULL)
                *result = completed_threads->tail->func_return;
            break;
        }
        else { //keep looping through thread_pool until thread exits
            interruptEnable();
            threadYield();
            interruptDisable();
        }
    }
    interruptEnable();
}

/*
    This is a wrapper function that executes a threads function (funcPtr)
    and passes in the argument (argPtr). This function allows us to get
    the return value of the threads function and pass that to threadExit.
*/
void threadReturn() {
    //get threads return value
    void *return_value =
        thread_pool->tail->funcPtr(thread_pool->tail->argPtr);
    //pass in return value and exit thread
    threadExit(return_value);
}

/*
    Exits the currently running thread (tail of thread_pool). If the
    main thread calls this function, the whole program will exit. The
    currently running thread's return value (func_return) is set equal
    to result for any calls to threadJoin by other running threads.
*/
void threadExit(void *result) {

    //remove running thread from the list
    interruptDisable();
    thread_data *running_thread = list_remove(thread_pool, TAIL);

    //set return value
    running_thread->func_return = result;

    //exit program if running_thread is main
    if (running_thread->ID == 0) {
        threadCleanup();
        exit(1);
    }

    //free the stack
    free(running_thread->context->uc_stack.ss_sp);

    //add thread to completed thread list
    list_insert(completed_threads, running_thread);

    //run next thread (if available)
    if (thread_pool->head != NULL) {
        thread_data *thread_to_run = list_remove(thread_pool, HEAD);
        list_insert(thread_pool, thread_to_run);
        interruptEnable();
        swapcontext(running_thread->context, thread_to_run->context);
        interruptDisable();
    }
    interruptEnable();
}

/*
    Deallocates all allocated memory within this file, such as a
    thread's context, stack, the thread itself and all three lists
    created in threadInit. **It does not free any memory allocated
    by the user, such as the result pointer passed in to a call to
    threadJoin.
*/
void threadCleanup() {
    //cleanup thread pool
    thread_data *thread;
    int i, j, k, num_threads = thread_pool->num_curr_threads;
    for (i = 0; i < num_threads; i++) {
        thread = list_remove(thread_pool, HEAD);
        if (thread->ID != 0)
            free(thread->context->uc_stack.ss_sp);
        free(thread->context);
        free(thread);
    }
    free(thread_pool);

    //cleanup completed threads
    num_threads = completed_threads->num_curr_threads;
    for (i = 0; i < num_threads; i++) {
        thread = list_remove(completed_threads, HEAD);
        free(thread->context);
        free(thread);
    }
    free(completed_threads);

    //cleanup waiting threads
    for (i = 0; i < NUM_LOCKS; i++) {
        for (j = 0; j < CONDITIONS_PER_LOCK; j++) {
            for (k = 0; k < waiting_threads[i][j]->num_curr_threads; k++) {
                thread = list_remove(waiting_threads[i][j], HEAD);
                free(thread->context->uc_stack.ss_sp);
                free(thread->context);
                free(thread);
            }
            free(waiting_threads[i][j]);
        }
    }
}

/*
    Waits until the currently running thread is able to acquire
    the lock associated with lockNum. It allows other threads to
    execute concurrently while this thread waits on the lock.
*/
void threadLock(int lockNum) {
    interruptDisable();
    while (locks[lockNum] != 0) {
        interruptEnable();
        threadYield();
        interruptDisable();
    }
    locks[lockNum] = 1;
    interruptEnable();
}

/*
    Unlocks the lock associated with lockNum (if it is locked).
*/
void threadUnlock(int lockNum) {
    interruptDisable();
    if (locks[lockNum] == 1)
        locks[lockNum] = 0;
    interruptEnable();
}

/*
    Atomically unlocks the mutex lock associated with lockNum and causes the
    current thread to wait on a condition (specified by conditionNum) to be
    signaled through a call to threadSignal. If the mutex lock is not locked
    when this function is called, the program exits with an error.
*/
void threadWait(int lockNum, int conditionNum) {
    interruptDisable();
    //if mutex is not locked before calling this function
    //the program prints an error and exits
    if (locks[lockNum] != 1) {
        fprintf(stderr, "Error: Mutex with number %d not locked.\n", lockNum);
        exit(-1);
    }
    else {
        interruptEnable();
        threadUnlock(lockNum);
        interruptDisable();
    }
    //add current thread to waiting list
    thread_data *running_thread = list_remove(thread_pool, TAIL);
    list_insert(waiting_threads[lockNum][conditionNum], running_thread);

    //run next thread (if available)
    if (thread_pool->head != NULL) {
        thread_data *thread_to_run = list_remove(thread_pool, HEAD);
        list_insert(thread_pool, thread_to_run);
        interruptEnable();
        swapcontext(running_thread->context, thread_to_run->context);
        interruptDisable();
    }

    //acquire lock again
    interruptEnable();
    threadLock(lockNum);
}

/*
    Removes a thread wating on a condtion variable specified by
    conditionNum from its list. If there is no thread waiting
    the function returns.
*/
void threadSignal(int lockNum, int conditionNum) {
    interruptDisable();
    if (waiting_threads[lockNum][conditionNum]->head == NULL) {
        interruptEnable();
        return;
    }
    else {
        thread_data *waiting_thread =
                list_remove(waiting_threads[lockNum][conditionNum], HEAD);
        list_insert(thread_pool, waiting_thread);
    }
    interruptEnable();
}

/*
    A wrapper function that disables interrupts. It checks to make
    sure that interrupts are enabled first before disabling them.
*/
static void interruptDisable () {
    assert(!interruptsAreDisabled);
    interruptsAreDisabled = 1;
}

/*
    A wrapper function that enables interrupts. It checks to make
    sure that interrupts are disabled first before enabling them.
*/
static void interruptEnable () {
    assert(interruptsAreDisabled);
    interruptsAreDisabled = 0;
}
