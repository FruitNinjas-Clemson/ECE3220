#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "list.h"

#define ITERATIONS 750000

/* prototype for list debug print */
void list_debug_print(thread_list *list_ptr);

int main(void)
{
	//construct list
	thread_list *list_ptr = list_construct();

	//random set of inserts and removes
	int i, r;
	time_t t;
	srand(time(&t));
	for (i = 0; i < ITERATIONS; i++) {

		r = rand() % 50;

		//insert
		if (r <= 25) {
			//malloc space for new thread and initialize values
			thread_data *new_thread = (thread_data *) malloc(sizeof(thread_data));
			new_thread->next = NULL;

			//insert into list
			list_insert(list_ptr, new_thread);
			if (list_ptr->head->next != NULL)
				assert(list_ptr->head < list_ptr->head->next);
		}
		//remove
		else  list_remove(list_ptr);
	}

	list_debug_print(list_ptr);

    return 0;
}

/*

Next you will want to write your own list_debug_print function to print a
list. Then you can do "before and after" testing. That is, print the list
before a change, and print the list after the change to verify that the change
worked.

*/

void list_debug_print(thread_list *list_ptr) {
	thread_data *rover = list_ptr->head;
	fprintf(stderr, "List:");
	while (rover != NULL) {
		if (rover->next == NULL)
			fprintf(stderr, " %p", rover);
		else
			fprintf(stderr, " %p -->", rover);
		rover = rover->next;
	}
	fprintf(stderr, "\n");
}
