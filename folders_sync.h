#ifndef FOLDERS_SYNC_H
#define FOLDERS_SYNC_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define COPY_BUFFER_SIZE 4096

// path and filename concatenation
void make_path(char *full_path, char *src_path, char *file_name);

// copy source file content to destination file
int file_copy(char *src_path, char *rep_path, FILE *log_file);

// check if path entry is directory
int is_dir(char *path);

// get time string
char *get_time_str();

// recursively remove contents of given directory
int delete_dir(char *path, FILE *log_file);

// check if directory on path exists
int dir_not_exist(char *path);

// main function for syncronization
int sync_folders(char *src_paths, char *rep_path, char *log_path);

// print message on stdout and to log file
void print_message(FILE *log_file, char *message, char *path);

// close log file, original and replica directories
void close_all(DIR *src_dir, DIR *rep_dir, FILE *log_file);

#endif
