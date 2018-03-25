/*
 * Austin Johnson
 * list.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include "list.h"
#include "allocator.h"

//global lists
list_t segregated_lists[NUM_FREE_LISTS];
list_t allocated_lists[NUM_ALLOC_LISTS];

/*
    Initializes the segregated and allocated linked lists. It
    allocates a page for each segregated list, where each node
    in the list is a fixed size (2, 4, 8, 16... 1024).
*/
void list_construct() {
    int i;
    for (i = 0; i < NUM_FREE_LISTS; i++)
        get_a_page(i);
}

/*
    Inserts a node onto the head of a list.
*/
void list_insert(list_t *list_ptr, list_node_t *node) {

    //null out any ptrs from previous lists
    node->prev = node->next = NULL;

    //insert into empty list
    if (list_ptr->head == NULL)
        list_ptr->head = node;

    //insert at the head of the list
    else {
        list_ptr->head->prev = node;
        node->next = list_ptr->head;
        list_ptr->head = node;
    }
}

/*
    Removes the head from a list and returns it.
*/
list_node_t *list_pop(list_t *list_ptr) {

		//if list is empty
		if (list_ptr->head == NULL) return NULL;

        //if only one item in the list
        if (list_ptr->head->next == NULL) {
            list_node_t *old_head = list_ptr->head;
            list_ptr->head = NULL;
            return old_head;
        }

        //remove the head
        else {
            list_node_t *old_head = list_ptr->head;
            old_head->next->prev = NULL;
            list_ptr->head = old_head->next;
            return old_head;
        }
}

/*
    Searches an allocated lists for a node and removes
    it from its list

*/
int list_remove (list_t *list_ptr, list_node_t *node) {

    //if list is empty
    if (list_ptr->head == NULL) return 0;

    //if only one item in the list
    else if (list_ptr->head->next == NULL) {
        if (list_ptr->head == node) {
            list_ptr->head = NULL;
            return 1;
        }
        else
            return 0;
    }
    //remove the head
    else if (list_ptr->head == node) {
        list_node_t *new_head = list_ptr->head->next;
        new_head->prev = NULL;
        list_ptr->head = new_head;
        return 1;
    }
    //search for node
    else {
        list_node_t *rover = list_ptr->head;
        while (rover != NULL) {
            //if its the tail
            if (rover == node && rover->next == NULL) {
                rover->prev->next = NULL;
                return 1;
            }
            //in the middle
            if (rover == node) {
                rover->next->prev = rover->prev;
                rover->prev->next = rover->next;
                return 1;
            }
            rover = rover->next;
        }
    }
    return 0;
}

/*
    A debugging function that prints out the nodes in a list.
*/
void list_debug_validate(list_t *list_ptr) {
    list_node_t *rover = list_ptr->head;
    while(rover != NULL) {
        if (rover == list_ptr->head)
            fprintf(stderr, "\n%p", rover);
        else
            fprintf(stderr, " -> %p", rover);
        rover = rover->next;
    }
    fprintf(stderr, "\n");
}
