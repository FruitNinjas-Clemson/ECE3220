/*
    Austin Johnson and Hunter Booth
    ECE 3320
    notjustcats.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "file.h"


//global root directory
directory_t *root_dir;

//pointer to the beginning of the FAT
char *FAT;

/*
    Function to get the size of an image (in bytes)
*/
long get_image_size(char *image) {
    //get image size
    FILE *fp = fopen(image, "rb");
    fseek(fp, 0L, SEEK_END);
    long fsize = ftell(fp);
    fclose(fp);
    return fsize;
}

/*
    This function allocates memory for a file
    and copies its contents into a string in
    order to allow for easy traversal
*/
char *allocate_for_file(char *filename) {
    //allocate for image and copy contents
    long fsize = get_image_size(filename);
    char *file_string = (char *) calloc(fsize + 1, sizeof(char));
    FILE *fp = fopen(filename, "rb");
    fread(file_string, fsize, 1, fp);
    fclose(fp);
    file_string[fsize] = '\0';
    return file_string;
}

/*
    Function to get the size of a file from the
    directory entry (in bytes)
*/
long get_file_size (char *entry) {
    //file size starts at offset 28 in the
    //directory and is 4 bytes in length
    char size_as_string[9];

    //get each individual byte
    int entry_31 = *(entry + 31) & 0xFF;
    int entry_30 = *(entry + 30) & 0xFF00 >> 8;
    int entry_29 = *(entry + 29) & 0xFF0000 >> 16;
    int entry_28 = *(entry + 28) & 0xFF000000 >> 24;

    //convert from little endian and get the file size
    sprintf(size_as_string, "%02x%02x%02x%02x", entry_31, entry_30, entry_29, entry_28);
    long filesize = (long) strtol(size_as_string, NULL, 16);
    return filesize;
}

/*
    Function to get the first logical cluster
    from the directory entry
*/
int first_logical_cluster (char *entry) {

    char cluster_as_string[5];

    //get individual bytes
    int entry_27 = *(entry + 27);
    int entry_26 = *(entry + 26);

    //convert from little endian and get first cluster
    sprintf(cluster_as_string, "%02x%02x", entry_27, entry_26);
    int cluster = (int) strtol(cluster_as_string, NULL, 16);
    return cluster;
}

/*
    Function to get the next logical cluster from
    the FAT
*/
int next_cluster_value (int FAT_entry) {
    int offset = 3;
    short next_cluster = 0;

    //get the offset for the FAT
    offset += ((FAT_entry / 2) - 1) * 3;
    offset += FAT_entry % 2;

    //move pointer to correct location
    memcpy(&next_cluster, FAT + offset, 2);

    //mask correct bits and shift accordingly
    if (FAT_entry % 2)
        next_cluster = (next_cluster & 0xFFF0) >> 4;
    else
        next_cluster = next_cluster & 0x0FFF;

    return next_cluster;
}

/*
    Function to calculate the sector number from
    a cluster value
*/
int sector_number (int cluster_value) {
    return 33 + cluster_value - 2;
}

/*
    Function to concatenate a static string onto
    a dynamic string that represents the path of
    a file
*/
char *get_path (char *curr_path, char *filename, char *extension) {
    char *path = (char *) calloc(strlen(curr_path)+strlen(filename)+
        strlen(extension)+1, sizeof(char));
    strcpy(path, curr_path);
    strcat(path, filename);
    strcat(path, extension);
    return path;
}

/*
    Function to trim the whitespace of a string
*/
void trim (char *string) {
    char *i = string, *j = string;
    while (*j != '\0') {
        *i = *j++;
        if (*i != ' ')
            i++;
    }
    *i = 0;
}

/*
    Function to get the file info contained in a directory
*/
void get_file_info(directory_t *dir_ptr, char *file_string, int offset, char *path) {
    int i, j, num_files = 0;
    char filename[9], extension[4];

    //count number of entries in directory
    char *rover = file_string + offset;
    while (strcmp(rover, "")) {
        num_files++;
        rover += 32; //each entry is 32 bytes long
    }

    //get information about each entry
    int directory_end = offset + (num_files * 32);
    for (i = offset, j = 0; i < directory_end; i += 32, j++) {

        char *dir_start = file_string + i;

        //file name starts at offset 0 and is 8 bytes
        memcpy(filename, dir_start, 8);
        if (*dir_start == DELETED)
            *filename = '_';
        filename[8] = '\0', trim(filename);

        //extension starts at offset 8 and is 3 bytes
        memcpy(extension, dir_start + 8, 3);
        extension[3] = '\0';

        //continue past parent directories
        if (*dir_start == '.') {
            num_files--;
            continue;
        }

        //if the entry has no extension it is a directory
        if (strcmp(extension, "   ") == 0) {

            //don't count directory as a file
            num_files--;

            //allocate for new directory
            directory_t *new_dir = (directory_t *) malloc(sizeof(directory_t));
            new_dir->num_files = 0;
            new_dir->path = (char *) calloc(strlen(path)+strlen(filename)+2, sizeof(char));
            strcpy(new_dir->path, path);
            strcat(new_dir->path, filename);
            strcat(new_dir->path, "/");
            new_dir->fhead = new_dir->ftail = NULL;
            new_dir->dhead = new_dir->dtail = NULL;
            new_dir->next = NULL;

            //get the data of the sector from the first logical cluster
            int first_cluster = first_logical_cluster(dir_start);
            int sector = sector_number(first_cluster);
            new_dir->data_of_sector = sector * DATA_SIZE;

            //add the new directory to the root directory list
            directory_insert(new_dir);
        }

        //else the entry must be a file
        else {
            //allocate new file
            file_t *new_file = (file_t *) malloc(sizeof(file_t));
            new_file->path = new_file->data = NULL; new_file->next = NULL;

            ///check if the file is normal or deleted
            if (*(dir_start) == DELETED)
                strcpy(new_file->status, "DELETED");
            else
                strcpy(new_file->status, "NORMAL");

            //assign filename and extension
            sprintf(new_file->filename, "%s", filename);
            sprintf(new_file->extension, ".%s", extension);

            //first logical cluster starts at offset 26 and is 2 bytes
            new_file->current_cluster = first_logical_cluster(dir_start);

            //file size starts at offset 28 and is 4 bytes
            new_file->filesize = get_file_size(dir_start);
            new_file->data = (char *) calloc(new_file->filesize + 1, sizeof(char));

            //get the path of the file
            new_file->path = get_path(dir_ptr->path, new_file->filename, new_file->extension);

            file_insert(dir_ptr, new_file);
        }
    }
    dir_ptr->num_files += num_files;
}

/*
    Function to get the data of all files under a directory
*/
void get_data (directory_t *dir_ptr, char *file_string) {

    //first get the files under the directory
    get_file_info(dir_ptr, file_string, dir_ptr->data_of_sector, dir_ptr->path);

    //get the data of a each file
    int i; file_t *file = dir_ptr->fhead;
    for (i = 0; i < dir_ptr->num_files; i++) {
        int i, bytes_to_write = 0, bytes_written = 0;
        while (file->current_cluster != END && file->current_cluster != FREE) {
            //get the next cluster
            int next_cluster = next_cluster_value(file->current_cluster);

            //get the data section of the sector
            int sector = sector_number(file->current_cluster);
            int data_of_sector = sector * DATA_SIZE;

            //get how many bytes to write to data buffer
            if (file->filesize - bytes_written < DATA_SIZE)
                bytes_to_write = file->filesize - bytes_written;
            else
                bytes_to_write = DATA_SIZE;

            //write bytes to buffer
            for (i = 0; i < bytes_to_write; i++) {
                sprintf(file->data + bytes_written + i, "%c", *(file_string + data_of_sector + i));
            }
            bytes_written += bytes_to_write;
            file->current_cluster = next_cluster;
        }
        file = file->next;
    }
}

/*
    Function to output to stdout the files found under a directory
    as well as output their contents to a specified output directory
*/
void output_files(directory_t *dir_ptr, char *output_directory, int *file_num) {
    int i;
    file_t *file = file_pop(dir_ptr);
    while (file != NULL) {

        //output to stdout
        fprintf(stdout, "FILE\t%s\t%s\t%ld\n",
        file->status, file->path, file->filesize);

        //count how many digits the file number contains
        //in order to allocate a correctly sized buffer
        int n = *file_num;
        int length_file_num = 1;
        while (n != 0) {
            n /= 10;
            ++length_file_num;
        }

        //output to the specified directory
        char *buffer = calloc(strlen(output_directory) + strlen("/file") + length_file_num + strlen(file->extension) + 1, sizeof(char));
        sprintf(buffer, "%s/file%d%s", output_directory, *file_num, file->extension);
        FILE *fp = fopen(buffer, "wb");
        for (i = 0; i < file->filesize; i++) {
            fwrite(file->data + i, 1, sizeof(file->data[i]), fp);
        }
        fclose(fp);

        //cleanup
        free(buffer);
        free(file->path);
        free(file->data);
        free(file);

        //get the next file
        file = file_pop(dir_ptr);
        *file_num = *file_num + 1;
    }
}

/*
    A debugging funtion to confirm a file's
    contents were copied correctly
*/
void file_debug_print(char *file, int fsize) {
    int i;
    for (i = 0; i < fsize; i++)
        fprintf(stderr, "%c", file[i]);
    fprintf(stderr, "\n");
}

/*
    This function constructs a root directory structure and
    allocates the image in argument 1 as a string
*/
char *construct (char *filename) {
    char *file_string = allocate_for_file(filename);
    root_dir = (directory_t *) malloc(sizeof(directory_t));
    root_dir->num_files = 0;
    root_dir->data_of_sector = ROOT;
    root_dir->path = (char *) calloc(2, sizeof(char));
    strcpy(root_dir->path, "/");
    root_dir->fhead = root_dir->ftail = NULL;
    root_dir->dhead = root_dir->dtail = NULL;
    root_dir->next = NULL;
    //get the start of the fat
    FAT = file_string + FAT_START;
    return file_string;
}

int main(int argc, char *argv[]) {

    //allocate for image
    char *file_string = construct(argv[1]);

    //get the data of the all the files under the root directory
    //and output each file to the specified directory
    int *file_num = (int *) malloc(sizeof(int)); *file_num = 0;
    get_data(root_dir, file_string);
    output_files(root_dir, argv[2], file_num);

    //get the data of all of the files under the sub directories
    //and output each file to the specifed directory
    directory_t *sub_directory = directory_pop();
    while (sub_directory != NULL) {
        get_data(sub_directory, file_string);
        output_files(sub_directory, argv[2], file_num);

        //cleanup sub directory
        free(sub_directory->path);
        free(sub_directory);

        sub_directory = directory_pop();
    }

    //cleanup all other memory
    free(file_num);
    free(file_string);
    free(root_dir->path);
    free(root_dir);

    return 0;
}
