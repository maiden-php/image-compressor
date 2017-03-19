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
#include <sys/mman.h>

#include "ftree.h"
#include "hash.h"

#define BUFFER_SIZE 1024

// Use a shared variable to flag errors.
// The reason why we use this is to allow for us to keep copying when there are errors in our child processes.
// Because using negative exit codes is not reliable (since exit codes are in the range 0-255) it would be hard
// to track how many child processes were created.
static int *error;

// Copies a file from source to destination.
int copy_file(const char* src, const char* dest, mode_t permissions)
{
    FILE *src_file;
    FILE *dest_file;
    char buffer[BUFFER_SIZE];

    // Open source file for reading.
    if ((src_file = fopen(src, "rb")) == NULL)
    {
        perror("Error opening source file");
        return -1;
    }

    // Open destination file for writing.
    if ((dest_file = fopen(dest, "wb")) == NULL)
    {
        fclose(src_file);
        perror("Error opening destination file");
        return -1;
    }

    // Error indicates if there was an error while copying the file.
    int error = -1;

    while (feof(src_file) == 0)
    {
        // Read chunk from source.
        size_t bytes;
        if ((bytes = fread(buffer, 1, sizeof(buffer), src_file)) != BUFFER_SIZE)
        {
            // check for errors
            if (ferror(src_file) != 0)
            {
                perror("Error reading file");
                goto close_with_error;
            }
        }
        // Write it to dest.
        size_t bytes_written = fwrite(buffer, 1, bytes, dest_file);

        // Check for errors.
        if (bytes_written < 0)
        {
            perror("Error writing to file");
            goto close_with_error;
        }
    }

    // Set permissions on file.
    if (fchmod(fileno(dest_file), permissions) != 0)
    {
        perror("Error setting permissions on file");
        goto close_with_error;
    }

    // If we reached this point means we have no errors.
    error = 0;

close_with_error:

    // Close files.
    fclose(src_file);
    fclose(dest_file);

    return error;
}

/* Create a directory with set permissions. If directory exists just set the permissions.
* Returns 0 on success, -1 otherwise.
*/
int create_directory(char *path, int permissions)
{
    // Try to create a directory.
    if (mkdir(path, permissions) != 0)
    {
        // If directory already exists
        if (errno == EEXIST)
        {
            // Set the permissions
            if (chmod(path, permissions) != 0)
            {
                perror("Error setting permissions on directory");
                return -1;
            }
        }
        else
        {
            perror("Error creating directory");
            return -1;
        }
    }

    return 0;
}

/* Recursively copy the source directory contents into the destination directory creating child processes for each sub directory.
 * Returns the number of processes used in the copy.
 */
int copy_directory(const char *src, const char *dest)
{
    int num_processes = 1;

    // Open the source directory.
    DIR *dir = opendir(src);

    // Check for error.
    if (dir == NULL)
    {
        perror("Error opening directory");
        return -1;
    }

    // Iterate over each file/directory/link in our current source directory.
    struct dirent *sd;
    while ((sd = readdir(dir)) != NULL)
    {
        // Skip . and ..
        if (!strcmp(sd->d_name, ".") || !strcmp(sd->d_name, ".."))
        {
            continue;
        }

        // Build source path.
        char *src_path = malloc(sizeof(char) * (strlen(src) + strlen(sd->d_name) + 1));
        if (src_path == NULL)
        {
            perror("Error allocating memory.");
            *error = -1;
            continue;
        }
        strcpy(src_path, src);
        strcat(strcat(src_path, "/"), sd->d_name);


        // Build destination path.
        char *dest_path = malloc(sizeof(char) * (strlen(dest) + strlen(sd->d_name) + 1));
        if (dest_path == NULL)
        {
            perror("Error allocating memory.");
            *error = -1;
            continue;
        }
        strcpy(dest_path, dest);
        strcat(strcat(dest_path, "/"), sd->d_name);


        // Retrieve information from the file / directory / symlink.
        struct stat src_st;
        if (stat(src_path, &src_st) < 0)
        {
            perror("Error reading information about source file");
            *error = -1;

            // Skip everything else except freeing memory for the source and destination paths.
            goto free_memory;
        }

        // Check if current entry is a regular file.
        if (S_ISREG(src_st.st_mode))
        {
            // Retrieve information about the destination file to compare their lengths and permissions.
            struct stat dest_st;
            if (stat(dest_path, &dest_st) < 0 && errno != ENOENT)
            {
                perror("Error reading information about destination file");
                *error = -1;

                // Skip everything else except freeing memory for the source and destination paths.
                goto free_memory;
            }
            
            // Check if the file exists in the destination directory.
            if (errno == ENOENT)
            {
                // File doesn't exist - copy it.
                if (copy_file(src_path, dest_path, src_st.st_mode) < 0)
                {
                    *error = -1;
                }
            }
            else
            {
                // If files don't have the same size we should just copy them and skip the hash calculation
                // which can be slow.
                if (src_st.st_size != dest_st.st_size)
                {
                    // Sizes don't match - copy the file.
                    if (copy_file(src_path, dest_path, src_st.st_mode) < 0)
                    {
                        *error = -1;
                    }
                }
                else
                {
                    // Retrieve file hashes for comparison.
                    char *src_hash = hash_file_name(src_path);
                    char *dest_hash = hash_file_name(dest_path);

                    // Source hash has to exist but destination hash might not, that's why we only check for
                    // errors on the source hash.
                    if (src_hash == NULL)
                    {
                        perror("Error reading hash from source");
                        *error = -1;
                    }
                    // Compare hashes to see if they match.
                    else if (compare_hashes(src_hash, dest_hash) != 0)
                    {
                        // copy the file
                        if (copy_file(src_path, dest_path, src_st.st_mode) < 0)
                        {
                            *error = -1;
                        }
                    }
                    // Compare the permissions to see if destination needs to be updated.
                    else if (src_st.st_mode != dest_st.st_mode)
                    {
                        // Set permissions on file.
                        if (chmod(dest_path, src_st.st_mode) != 0)
                        {
                            perror("Error setting permissions on file");
                        }
                    }


                    // Release memory allocated for hashes.
                    free(src_hash);
                    free(dest_hash);
                }
            }
        }
        // Check if it's a directory.
        else if (S_ISDIR(src_st.st_mode))
        {
            // It is a directory so we create a child process to recursively call copy_directory and perform
            // the copy of the sub directories.
            int pid = fork();
            
            // Check for error creating child process.
            if (pid == -1)
            {
                perror("Error creating child process");
                *error = 1;
            }
            // Check if this is the child process.
            else if (pid == 0)
            {
                // Create a directory with same permissions as source directory.
                if (create_directory(dest_path, src_st.st_mode) != 0)
                {
                    *error = -1;
                    exit(num_processes);
                }
                
                // Perform the recursive copy of the sub directory.
                exit(copy_directory(src_path, dest_path));
            }
            else
            {
                // If not child then it's the parent process.

                // Wait on child and checks for error codes.
                int status = 0;
                if (wait(&status) == -1)
                {
                    perror("Error waiting for child");
                    *error = -1;
                }
                
                // Update the number of child processes.
                num_processes += WEXITSTATUS(status);
            }
        }

free_memory:
        // Release strings allocated for source and destination paths.
        free(src_path);
        free(dest_path);
    }

    if (closedir(dir) != 0)
    {
        perror("Error closing directory");
        *error = -1;
    }

    return error >=0 ? num_processes : -num_processes;
}


/* Function for copying a file tree rooted at src to dest
* Returns < 0 on error. The magnitude of the return value
* is the number of processes involved in the copy and is
* at least 1.
*/
int copy_ftree(const char *src, const char *dest)
{
    // Read information about source directory.
    struct stat src_st;
    stat(src, &src_st);

    // Build name of destination path where to copy the contents of source.
    // If src is /a/b/c and dest is /t then the result should be something like: dest/base(src) which is /t/c .
    char *base_src = strdup(basename((char*)src));
    char *dest_path = malloc(sizeof(char) * (strlen(dest) + strlen(src) + 1));
    strcpy(dest_path, dest);
    strcat(strcat(dest_path, "/"), base_src);
    
    // Create source directory inside destination directory and keep its permissions.
    if (create_directory(dest_path, src_st.st_mode) != 0)
    {
        return -1;
    }

    // Initialize shared memory to set error flag.
    error = mmap(NULL, sizeof *error, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (error == MAP_FAILED)
    {
        perror("Error creating shared memory");
        return -1;
    }

    // No errors to begin with.
    *error = 0;

    // Perform the copy of the source to destination directory.
    int num_processes = copy_directory(src, dest_path);

    // Free strings allocated for paths.
    free(base_src);
    free(dest_path);

    // Update our number of processes by checking if any child reported an error.
    num_processes *= (*error >= 0 ? 1 : -1);

    // Free shared memory.
    munmap(error, sizeof *error);

    return num_processes;
}

#endif // _FTREE_H_
