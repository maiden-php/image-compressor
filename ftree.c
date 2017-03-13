#ifndef _FTREE_H_
#define _FTREE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <errno.h>
#include <libgen.h>

#include "ftree.h"
#include "hash.h"

#define BUFFER_SIZE 1024


// Copies a file from source to destination
int copy_file(const char* src, const char* dest, mode_t permissions)
{
    FILE *srcFile;
    FILE *destFile;
    char buffer[BUFFER_SIZE];
    size_t bytes, bytesWritten;


    // open source file for reading
    if ((srcFile = fopen(src, "rb")) == NULL) {
        fprintf(stderr, "Error opening source file: %s\n", src);
        return -1;
    }

    // open destination file for writing
    if ((destFile = fopen(dest, "wb")) == NULL) {
        fprintf(stderr, "Error opening destination file: %s\n", dest);
        return -1;
    }

    while (feof(srcFile) == 0) {
        // read chunk from source
        if ((bytes = fread(buffer, 1, sizeof(buffer), srcFile)) != BUFFER_SIZE) {
            // check for errors
            if (ferror(srcFile) != 0) {
                fprintf(stderr, "Error reading file.\n");
                return -1;
            }
        }
        // write it to dest
        bytesWritten = fwrite(buffer, 1, bytes, destFile);

        // check for errors
        if (bytesWritten < 0) {
            fprintf(stderr, "Error writing to file.\n");
            return -1;
        }
    }

    // set permissions on file
    fchmod(fileno(destFile), permissions);

    // close files
    fclose(srcFile);
    fclose(destFile);

    return 0;
}

// copy source folder contents into dest folder creating child processes for each sub folder.
int copy_folder(const char *src, const char *dest)
{
    struct stat srcSt;
    struct stat destSt;
    int childStatus;
    int numProcesses = 1;
    int statchmod;

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
        char *srcPath = malloc(sizeof(char) * (strlen(src) + 2 + strlen(sd->d_name)));
        strcpy(srcPath, src);
        strcat(srcPath, "/");
        strcat(srcPath, sd->d_name);

        // destination path
        char *destPath = malloc(sizeof(char) * (strlen(dest) + 2 + strlen(sd->d_name)));
        strcpy(destPath, dest);
        strcat(destPath, "/");
        strcat(destPath, sd->d_name);

        // gets all the information from the file, dir, or symlink
        stat(srcPath, &srcSt);

        // if it's a regular file
        if (S_ISREG(srcSt.st_mode)) {
            // grab the size of the destination and check if we need to copy the file
            stat(destPath, &destSt);
            if (errno != ENOENT) {
                char *srcHash = hash_by_filename(srcPath);
                char *destHash = hash_by_filename(destPath);

                // check file size and hash
                if ((srcSt.st_size != destSt.st_size) || compare_hashes(srcHash, destHash)) {
                    
                    // copy the file
                    copy_file(srcPath, destPath, srcSt.st_mode);
                }

                free(srcHash);
                free(destHash);
            }
            else
            {
                // file doesn't exist in target path so just copy it
                copy_file(srcPath, destPath, srcSt.st_mode);
            }
        }
        else if (S_ISDIR(srcSt.st_mode)) { 	// if it's a directory, 
            // create a child process
            int childPid = fork();
            if (childPid == 0) {
                // the child process is responsible for creating a folder and copying the contents inside that folder.

                // keep the permissions
                statchmod = srcSt.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
                mkdir(destPath, statchmod);
                exit(copy_folder(srcPath, destPath));
            }
            else
            {
                // the parent process waits on the child
                wait(&childStatus);
                if (childStatus > 0)
                    numProcesses += childStatus / 256;
            }
        }



        // release memory for the allocated source and dest path strings
        free(srcPath);
        free(destPath);
    }

    closedir(dir);

    return numProcesses;
}


/* Function for copying a file tree rooted at src to dest
* Returns < 0 on error. The magnitude of the return value
* is the number of processes involved in the copy and is
* at least 1.
*/
int copy_ftree(const char *src, const char *dest)
{
    // read information about source folder
    struct stat srcSt;
    stat(src, &srcSt);
    int statchmod = srcSt.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);


    // build name of destination path where to copy the contents of source
    // if src is /a/b/c and dest is /t then the result should be something like: dest/base(src) which is /t/c
    char *baseSrc = strdup(basename((char*)src));
    char *destPath = malloc(sizeof(char) * strlen(dest) + strlen(src) + 1);
    strcpy(destPath, dest);
    strcat(destPath, "/");
    strcat(destPath, baseSrc);
    // create source folder inside dest folder
    mkdir(destPath, statchmod);


    int numProcesses = copy_folder(src, destPath);

    free(destPath);

    return numProcesses;

}

#endif // _FTREE_H_
