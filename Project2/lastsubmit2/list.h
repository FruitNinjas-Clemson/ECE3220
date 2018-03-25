/*
 * Austin Johnson
 * list.h
 */

#include <ucontext.h>
#include "mythreads.h"

//define two lock statuses
#define UNLOCKED 0
#define LOCKED 1

//define list removal locations
#define HEAD 0
#define TAIL 1

//initialize thread data structure
typedef struct thread_data_t {
	int ID;
	ucontext_t *context;
	thFuncPtr funcPtr;
	void *func_return;
	void *argPtr;
	struct thread_data_t *next;
} thread_data;

//initialize thread list data structure
typedef struct thread_list_t {
	int num_curr_threads;
	thread_data *head;
	thread_data *tail;
} thread_list;

//share thread_pool and completed_threads
//declared in mythreads.c between files
extern thread_list *thread_pool;
extern thread_list *completed_threads;

/* prototype definitions for functions in list.c */
thread_list *list_construct();
void list_insert(thread_list *list_ptr, thread_data *new_thread);
thread_data *list_remove(thread_list *list_ptr, int position);
void list_debug_validate(thread_list *list_ptr);
