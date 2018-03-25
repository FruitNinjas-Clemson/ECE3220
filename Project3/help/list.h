/*
 * Austin Johnson
 * list.h
 */

#define PAGE_SIZE 4096
#define NUM_FREE_LISTS 10

/* The number of allocated lists is 1 greater than
   the number of free lists for the irregular block
   sizes. Those blocks are not kept up with in a free
   list and instead are allocated on the spot.
*/
#define NUM_ALLOC_LISTS 11

//declare node structure
typedef struct list_node_tag {
    size_t block_size;
    struct list_node_tag *prev;
	struct list_node_tag *next;
} list_node_t;

//declare list structure
typedef struct list_tag {
	list_node_t *head;
} list_t;

//global lists
extern list_t segregated_lists[NUM_FREE_LISTS];
extern list_t allocated_lists[NUM_ALLOC_LISTS];

/* prototype definitions for functions in list.c */
void __attribute__ ((constructor)) list_construct(void);
void list_insert(list_t *list_ptr, list_node_t *node);
list_node_t *list_pop(list_t *list_ptr);
int list_remove (list_t *list_ptr, list_node_t *node);
void list_debug_validate(list_t *list_ptr);
