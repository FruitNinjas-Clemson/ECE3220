/*
 * Austin Johnson
 * allocator.h
 */

 /* prototype definitions for functions in allocator.c */
void *malloc(size_t numbytes);
void *realloc (void *ptr, size_t numbytes);
void *calloc (size_t numelem, size_t numbytes);
void free (void *ptr);
void get_a_page (int list_index);
