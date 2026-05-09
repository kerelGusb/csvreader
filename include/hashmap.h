#ifndef HASHMAP_H
#define HASHMAP_H

#include <time.h>

typedef struct Entry Entry_t;

struct Entry {
    char* key;
    int value;

    Entry_t* next;
};

typedef struct Hashmap {
    Entry_t** entries;
    size_t size;
    unsigned int seed;
} Hashmap_t;

unsigned int hash(const char* str);
Hashmap_t* hm_init(size_t size);
int hm_insert(Hashmap_t* hm, const char* key, int value);
Entry_t* hm_get(Hashmap_t* hm, const char* key);
void hm_free(Hashmap_t* hm);

#endif