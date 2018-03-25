/*
*  Austin Johnson
*  CPSC 3220 
*  Project 1
*  memory_shim.c
*/

//function prototypes
void __attribute__ ((constructor)) mem_construct (void);
void __attribute__ ((destructor)) mem_cleanup(void);

#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>


//function pointers
void *(*original_malloc)(size_t size) = NULL;
void (*original_free) (void *ptr) = NULL;


//structure for a block in a memory pool
typedef struct allocated_memory_block {
	size_t block_size;
	struct allocated_memory_block *next;
} memory_block;

//structure for a memory pool
typedef struct allocated_memory_list {
	size_t tot_bytes_leaked;
	int num_leaks;
	memory_block *head;
} memory_list;


//global memory pool
memory_list memory_pool;

/*Construct malloc and free*/
void mem_construct (void) {

	//initialize memory_pool
	memory_pool.tot_bytes_leaked = 0;
	memory_pool.num_leaks = 0;
	memory_pool.head = NULL;

	//assign function pointers
	if (original_malloc == NULL) 
		original_malloc = dlsym(RTLD_NEXT, "malloc");	
	if (original_free == NULL)
		original_free = dlsym(RTLD_NEXT, "free");
}

/*My Malloc*/
void *malloc (size_t size) {
	
	//allocate memory
	void *malloc_return = original_malloc(size);

	//if memory was allocated add to linked list
	if (malloc_return != NULL) { 
		if (memory_pool.head == NULL) {
			memory_pool.head = malloc_return;
			memory_pool.head->block_size = size;
			memory_pool.head->next = NULL;
		}
		else {
			memory_block *temp = memory_pool.head;
			memory_pool.head = malloc_return;
			memory_pool.head->block_size = size;
			memory_pool.head->next = temp;
		}
	}	
	return malloc_return;
}

/*My Free*/
void free (void *ptr) {

	//free memory
	if (memory_pool.head == NULL) return;
	else if (ptr == memory_pool.head) {
		memory_pool.head = 
		     memory_pool.head->next;		
		original_free(ptr);
	}
	else {
		memory_block *prev = NULL;
		memory_block *curr = memory_pool.head;
		while (curr->next != NULL && curr != ptr) {
			prev = curr;
			curr = curr->next;
		}
		//if freeing an invalid pointer
		if (curr != ptr) return;
		else {
			prev->next = curr->next;
			original_free(ptr);
			curr = NULL;	
		}
	}							
}


/* Print memory leaks and/or total allocations */
void print_memory_stats() { 
	memory_block *rover = memory_pool.head;
	while (rover != NULL) {
		memory_pool.tot_bytes_leaked += rover->block_size;
		memory_pool.num_leaks += 1;
		fprintf(stderr, "\nLEAK\t%zu", rover->block_size);
		rover = rover->next;
	}
	fprintf(stderr, "\nTOTAL\t%d\t%zu\n", 
		memory_pool.num_leaks, memory_pool.tot_bytes_leaked);
}


/*Cleanup Shared Library*/
void mem_cleanup(void) {
	print_memory_stats();
	
	//release memory not freed by the user
	memory_block *rover = memory_pool.head;
	memory_block *temp = NULL;
	while (rover != NULL) {
		temp = rover;
		rover = rover->next;
		free(temp);
	}
}
