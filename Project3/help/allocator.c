/*
 * Austin Johnson
 * allocator.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include "list.h"
#include "allocator.h"

//this variable declares the max amount of bytes
//that a user can request. It's primary use is to
//check against invalid free(s) and invalid realloc(s)
//i.e. pointers on the stack
#define MAX_BYTES UINT32_MAX

/*
    If the numbytes is greater than 0 and less than or equal
    to 1024, malloc removes the head from the corresponding
    segregated list and returns its data ptr. If numbytes is
    greater than 1024, malloc uses mmap to request for that
    amount of memory and returns its data ptr.
*/
void *malloc(size_t numbytes) {

    //invalid request
    if (numbytes < 1) return NULL;

    //if numbytes is greater than 1024, we must use mmap
    //for the block (as it is not a common size)
    int header_size = sizeof(list_node_t);
    list_node_t *chunk = NULL;
    if (numbytes > 1024) {
        chunk = mmap(NULL, numbytes + header_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        chunk->block_size = numbytes;
        chunk->prev = NULL;
        chunk->next = NULL;
        list_insert(&allocated_lists[NUM_ALLOC_LISTS-1], chunk);
    }
    //take out a node from a list according to the segregated
    //list number (where 0 is 2^1, 1 is 2^2 etc.)
    else {
        //check that numbytes is a power of 2 -> if not round
        //up to the nearest power of 2
        if (numbytes == 1) numbytes = 2;
        else if (PAGE_SIZE % numbytes != 0)
            numbytes = pow(2, ceil(log(numbytes)/log(2)));
        //get a node from a segregated list
        int seg_list_num = log(numbytes)/log(2) - 1;
        //allocate for another page if segregated list is low on memory
        if (segregated_lists[seg_list_num].head == NULL)
            get_a_page(seg_list_num);
        //remove node on from free list and add to allocated list
        chunk = list_pop(&segregated_lists[seg_list_num]);
        list_insert(&allocated_lists[seg_list_num], chunk);
    }
    return ((void *) chunk) + header_size;
}

/*
    Returns memory from malloc that is initialized to zero
*/
void *calloc (size_t numelem, size_t numbytes) {
    //malloc for memory
    int header_size = sizeof(list_node_t);
    void *mem_block = malloc(numelem*numbytes);
    int block_size = ((list_node_t *) (mem_block - header_size))->block_size;

    //zero out the memory returned from malloc
    mem_block = memset(mem_block, 0, block_size);
    return mem_block;
}

/*
    If the request is for a smaller block, realloc just
    returns the same block to the user. If it is for a
    greater size block, realloc allocates for that size,
    copies the data from the old block, frees the old
    block and return the new data ptr.
*/
void *realloc (void *ptr, size_t numbytes) {

    //like a malloc call
    if (ptr == NULL) {
        ptr = malloc(numbytes);
        return ptr;
    }

    //if realloc for a smaller size just return the
    //same block
    int header_size = sizeof(list_node_t);
    list_node_t *old_block = (list_node_t *) (ptr - header_size);
    int old_block_size = old_block->block_size;

    //invalid ptr
    if (old_block_size <= 1 || (old_block_size <= 1024 && PAGE_SIZE
          % old_block_size != 0) || old_block->block_size > MAX_BYTES)
        return NULL;

    //if realloc for a smaller size just return the
    //same block
    if (old_block_size > numbytes)
        return ptr;

    //allocate for a new block, copying the data from
    //the old block and freeing the old block
    void *new_block = malloc(numbytes);
    memcpy(new_block, ptr, old_block_size);
    free(ptr);
    return new_block;
}

/*
    Inserts allocated blocks back into their corresponding free list if
    the size is less than 1024 B. Unmaps ptrs bigger than 1024 B.
*/
void free(void *ptr) {
    //invalid ptr
    if (ptr == NULL) return;

    //get node header by subtracting header
    //size from the ptr
    int header_size = sizeof(list_node_t);
    list_node_t *node =  (list_node_t *) (ptr - header_size);
    int block_size = node->block_size;

    //invalid ptr
    if (block_size <= 1 || (block_size <= 1024 && PAGE_SIZE % block_size != 0)
            || node->block_size > MAX_BYTES)
        return;

    //If the block size is greater than 1024 we unmap this block instead
    //of adding it to a segregated list since it is not a commonly requested
    //block (2,4,8...1024)
    if (node->block_size > 1024) {
        int node_size = node->block_size + header_size;
        if (node->prev == NULL && node->next == NULL) {
            if (node != allocated_lists[NUM_ALLOC_LISTS-1].head)
                return; //invalid ptr
            else
                allocated_lists[NUM_ALLOC_LISTS-1].head = NULL;
        }
        else {
            if (list_remove(&allocated_lists[NUM_ALLOC_LISTS-1], node) == 0)
                return; //invalid ptr
        }
        munmap(node, node_size);
    }
    //remove node from the allocated list according to the
    //block size and add it to the segregated list
    else {
        int seg_list_num = log(node->block_size)/log(2) - 1;
        if (node->prev == NULL && node->next == NULL) {
            if (node != allocated_lists[seg_list_num].head)
                return; //invalid ptr
            else
                allocated_lists[seg_list_num].head = NULL;
        }
        else {
            if (list_remove(&allocated_lists[seg_list_num], node) == 0)
                return; //invalid ptr;
        }
        //insert node into free list
        list_insert(&segregated_lists[seg_list_num], node);
    }
}

/*
    Gets a page of memory for a segregated list, making sure to account
    for each node's header
*/
void get_a_page (int list_index) {
    //get information about the list
    int block_size = pow(2, list_index+1);
    int header_size = sizeof(list_node_t);
    int node_size = block_size + header_size;
    //this is good integer division -> don't want
    //to mmap for over a page
    int num_nodes = PAGE_SIZE / node_size;
    list_node_t *chunk = NULL;
    //allocate a page worth of nodes (number of nodes dependent
    //upon the size of each node)
    int i;
    for (i = 0; i < num_nodes; i++) {
        chunk = mmap(NULL, node_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        //on error
        if ((void *) -1 == chunk) {
            fprintf(stderr, "Could not map memory: %s\n", strerror(errno));
        }
        chunk->block_size = block_size;
        chunk->prev = NULL;
        chunk->next = NULL;
        list_insert(&segregated_lists[list_index], chunk);
    }
}
