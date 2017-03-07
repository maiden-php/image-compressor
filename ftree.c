#ifndef _FTREE_H_
#define _FTREE_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "dirent.h"

#include "ftree.h"
#include "hash.h"

#define BUFFER_SIZE 1024


/* Function for copying a file tree rooted at src to dest
 * Returns < 0 on error. The magnitude of the return value
 * is the number of processes involved in the copy and is
 * at least 1.
 */
int copy_ftree(const char *src, const char *dest)
{
    return copy_folder(src, dest);
}

int copy_folder(const char *src, const char *dest)
{
    struct stat srcSt;
    struct stat destSt;
    char *srcPath;
    char *destPath;

    DIR *dir;
    dir = opendir(src); // open source directory
    struct dirent *sd;

    if (dir == NULL) {
        printf("Error, unable to open directory \n");
        return -1;
    }

    // go through each entry in our folder
    while ((sd = readdir(dir)) != NULL) {

        // skip . and .. directories
        if (!strcmp(sd->d_name, ".") || !strcmp(sd->d_name, "..")) {
            continue;
        }

        // source path
        srcPath = malloc(sizeof(char) * (strlen(src) + 2 + strlen(sd->d_name)));
        strcpy(srcPath, src);
        strcat(srcPath, "/");
        strcat(srcPath, sd->d_name);

        // destination path
        destPath = malloc(sizeof(char) * (strlen(dest) + 2 + strlen(sd->d_name)));
        strcpy(destPath, dest);
        strcat(destPath, "/");
        strcat(destPath, sd->d_name);

        // gets all the information from the file, dir, or symlink
        stat(srcPath, &srcSt);

        // if it's a regular file
        if (S_ISREG(srcSt.st_mode)) {
            // grab the size of the destination and check if we need to copy the file
            stat(destPath, &destSt);
            if (srcSt.st_size != destSt.st_size || hash(srcPath) != hash(destPath)) {
                // copy the file
                copy_file(srcPath, destPath);
            }
        }
        else if (S_ISDIR(srcSt.st_mode)) { 	// if it's a directory, 
            //TODO
        }

        // release memory for the allocated source and dest path strings
        free(srcPath);
        free(destPath);
    }

    closedir(dir);
}

// Copies a file from source to destination
int copy_file(const char* src, const char* dest)
{
    FILE *srcFile;
    FILE *destFile;
    char buffer[BUFFER_SIZE];
    size_t bytes, bytesWritten;

    
    // open source file for reading
    if ((srcFile = fopen(src, "rb")) == NULL) {
        fprintf(stderr, "open read file error.\n");
        return -1;
    }

    // open destination file for writing
    if ((destFile = fopen(dest, "wb")) == NULL) {
        fprintf(stderr, "open write file error.\n");
        return -1;
    }

    while (feof(srcFile) == 0) {
        // read chunk from source
        if ((bytes = fread(buffer, 1, sizeof(buffer), srcFile)) != BUFFER_SIZE) {
            // check for errors
            if (ferror(srcFile) != 0) {
                fprintf(stderr, "read file error.\n");
                return -1;
            }
        }
        // write it to dest
        bytesWritten = fwrite(buffer, 1, bytes, destFile);

        // check for errors
        if (bytesWritten < 0) {
            fprintf(stderr, "write file error.\n");
            return -1;
        }
    }

    // close files
    fclose(srcFile);
    fclose(destFile);

    return 0;
}

#endif // _FTREE_H_
