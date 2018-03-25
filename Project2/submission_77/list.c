/*
 * Austin Johnson
 * list.c
 * Purpose: Helper functions for mythreads.c that
 * insert and remove from a two-way linked list
 */

#include <stdio.h>
#include <stdlib.h>
#include "list.h"

/*
    Inserts a thread into a two-way linked list. It always inserts
    onto the tail of the list, as this list is implemented as a queue.
*/
void list_insert(thread_list *list_ptr, thread_data *new_thread) {

    //empty list
    if (list_ptr->head == NULL) {
        list_ptr->head = new_thread;
		list_ptr->tail = new_thread;
    }

    //insert at the end of the list
    else {
        new_thread->next = NULL;
		list_ptr->tail->next = new_thread;
        list_ptr->tail = new_thread;
    }
    list_ptr->num_curr_threads++;
}

/*
    Removes a thread from a two-way linked list. It removes the thread
    from the head or tail of the list.
*/
thread_data *list_remove(thread_list *list_ptr, int position) {

		//if list is empty
		if (list_ptr->head == NULL) return NULL;

        //if only one item in the list
        else if (list_ptr->head->next == NULL) {
            thread_data *head = list_ptr->head;
			list_ptr->head = NULL;
            list_ptr->num_curr_threads = 0;
			return head;
		}

        //remove from the head
        else if (position == HEAD) {
			thread_data *old_thread = list_ptr->head;
        	list_ptr->head = list_ptr->head->next;

            //if only one item left in the list
			//then head and tail are the same
            if (list_ptr->head->next == NULL) {
                list_ptr->tail = list_ptr->head;
			}
            list_ptr->num_curr_threads--;
			return old_thread;
       }

       //remove from the tail
       else {
           //find new tail
           thread_data *rover = thread_pool->head;
           while (rover->next != thread_pool->tail)
               rover = rover->next;

           //assign new tail
           thread_data *old_thread = list_ptr->tail;
           rover->next = NULL;
           thread_pool->tail = rover;

           //if only one item left in the list
           //then head and tail are the same
           if (list_ptr->head->next == NULL) {
               list_ptr->head = list_ptr->tail;
           }
           list_ptr->num_curr_threads--;
           return old_thread;
       }
}

/*
    A debugging function that prints out the nodes in a list.
*/
void list_debug_validate(thread_list *list_ptr) {
    thread_data *rover = list_ptr->head;
    while(rover != NULL) {
        fprintf(stderr, "Thread %d\n", rover->ID);
        rover = rover->next;
    }
}
