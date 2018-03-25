/*
    Austin Johnson and Hunter Booth
    ECE 3220
    Project 4
    file.h
*/

#define FAT_START 0x200
#define ROOT 0x2600
#define DATA_SIZE 512
#define FREE 0x0
#define END 0xFFF
#define DELETED -27

typedef struct file_tag {
    char filename[9];
    char extension[5];
    long filesize;
    char *path;
    int current_cluster;
    char *data;
    char status[8];
    struct file_tag *next;
} file_t;

typedef struct directory_tag {
    int num_files;
    int data_of_sector;
    char *path;
    file_t *fhead;
    file_t *ftail;
    struct directory_tag *dhead; //only root directory uses this
    struct directory_tag *dtail; //only root directory uses this
    struct directory_tag *next;
} directory_t;

extern directory_t *root_dir;

/* prototype definitions for functions in file.c */
void directory_insert(directory_t *new_dir);
directory_t *directory_pop();
void file_insert(directory_t *dir_ptr, file_t *new_file);
file_t *file_pop();
void directory_debug_validate();
