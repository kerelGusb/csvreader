#include <time.h>
#include <string.h>
#include <malloc.h>
#include "hashmap.h"


unsigned int hash(const char* str) {
    // haha sixseven so funny
    unsigned int hash = 12167671;
    char c;

    while ((c = *str++))
        hash = hash * 31 + c;

    return hash;
}


Hashmap_t* hm_init(size_t size) {
    Hashmap_t* hm = malloc(sizeof(Hashmap_t));
    if (!hm) return NULL;

    hm->size = size;

    hm->entries = calloc(size, sizeof(Entry_t*));
    if (!hm->entries) {
        free(hm);
        return NULL;
    }

    return hm;
}


int hm_insert(Hashmap_t* hm, const char* key, int value) {
    Entry_t* exist_entry = hm_get(hm, key);

    // For csvreader: if we already have cell name in hm,
    // it means there is more than one way to parse it.
    // So, in this case, it's error and we have to return 1.
    if (exist_entry) {
        return 1;
    }

    unsigned int h = hash(key) % hm->size;

    Entry_t *new_entry = malloc(sizeof(Entry_t));
    if (!new_entry) return 1;

    new_entry->key = strdup(key);
    if (!new_entry->key) {
        free(new_entry);
        return 1;
    }

    new_entry->value = value;
    new_entry->next = hm->entries[h];

    hm->entries[h] = new_entry;

    return 0;
}


Entry_t* hm_get(Hashmap_t* hm, const char* key) {
    unsigned int h = hash(key) % hm->size;

    Entry_t* e = hm->entries[h];
    while (e) {
        if (strcmp(e->key, key) == 0)
            return e;
        e = e->next;
    }
    return NULL;
}


void hm_free(Hashmap_t* hm) {
    for (size_t i = 0; i < hm->size; ++i) {
        Entry_t *e = hm->entries[i];

        while (e) {
            Entry_t *next = e->next;

            free(e->key);
            free(e);

            e = next;
        }
    }

    free(hm->entries);
    free(hm);
}

