/*
    Austin Johnson and Hunter Booth
    ECE 3220
    Project 4
    list.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "file.h"

/*
    Inserts a new directory onto the tail of the
    root directory list
*/
void directory_insert(directory_t *new_dir) {
    //empty list
    if (root_dir->dhead == NULL) {
        root_dir->dhead = new_dir;
		root_dir->dtail = new_dir;
    }

    //insert at the end of the list
    else {
        new_dir->next = NULL;
		root_dir->dtail->next = new_dir;
        root_dir->dtail = new_dir;
    }

}

/*
    Inserts a new file onto the tail of a directory
    list
*/
void file_insert(directory_t *dir_ptr, file_t *new_file) {

    //empty list
    if (dir_ptr->fhead == NULL) {
        dir_ptr->fhead = new_file;
		dir_ptr->ftail = new_file;
    }

    //insert at the end of the list
    else {
        new_file->next = NULL;
		dir_ptr->ftail->next = new_file;
        dir_ptr->ftail = new_file;
    }
}

/*
    Removes the head directory off of the
    root directory list
*/
directory_t *directory_pop() {

    directory_t *directory = NULL;

    //empty list
    if (root_dir->dhead == NULL) return NULL;

    //only one directory in the list
    if (root_dir->dhead->next == NULL) {
        directory = root_dir->dhead;
        root_dir->dhead = NULL;
    }
    else {
        directory = root_dir->dhead;
        root_dir->dhead = root_dir->dhead->next;
    }

    return directory;
}

/*
    Removes the head file off of a directory
    list
*/
file_t *file_pop(directory_t *dir_ptr) {

    file_t *file = NULL;

    //empty list
    if (dir_ptr->fhead == NULL) return NULL;

    //only one file in the directory
    if (dir_ptr->fhead->next == NULL) {
        file = dir_ptr->fhead;
        dir_ptr->fhead = NULL;
    }
    else {
        file = dir_ptr->fhead;
        dir_ptr->fhead = dir_ptr->fhead->next;
    }

    return file;
}

/*
    A debugging function that prints out the files
    within a directory
*/
void directory_debug_validate(directory_t *dir_ptr) {
    file_t *rover = dir_ptr->fhead;
    while(rover != NULL) {
        if (rover->next == NULL)
            fprintf(stderr, "%s (%zd B)\n", rover->filename, rover->filesize);
        else
            fprintf(stderr, "%s (%zd B) -> ", rover->filename, rover->filesize);
        rover = rover->next;
    }
}
