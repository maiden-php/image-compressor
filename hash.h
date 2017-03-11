#ifndef _HASH_H_
#define _HASH_H_

// Hash manipulation helper functions
char *hash(FILE *f);

short compare_hashes(const char *src, const char *dest);

char *hash_by_filename(const char *fName);

#endif // _HASH_H_
