#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hash.h"

const int block_size = 8; // expecting something like 74 1c 00 11 22 33 44 55 as output

// Hash manipulation helper functions
char *hash(FILE *f)
{
    char line[4096];

    char *hash_val = malloc(sizeof(char) * block_size);
    for (int i = 0; i < block_size; i++) {
        hash_val[i] = '\0';
    }

    while (fgets(line, sizeof(line), f) != NULL) {

        int count = 0;
        for (int i = 0; i < strlen(line); i++) {
            //printf("%c", line[i]);

            hash_val[count] = hash_val[count] ^ line[i];
            count++;

            if (count == block_size) {
                count = 0;
            }
        }
    }


    return hash_val;
}

short compare_hashes(const char *src, const char *dest)
{
    return memcmp(src, dest, sizeof(char) * block_size);
}


char *hash_by_filename(const char *fName)
{
    FILE *f;
    char *res;
    
    // open file for hashing
    if ((f = fopen(fName, "rb")) == NULL) {
        fprintf(stderr, "Error opening destination file: %s\n", fName);
        return NULL;
    }

    // get hash from file
    res = hash(f);

    // close file
    fclose(f);

    return res;
}

