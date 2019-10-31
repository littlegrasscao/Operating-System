#ifndef hash_h
#define hash_h

#include "spin.h"

// basic node structure
typedef struct hash_node {
    unsigned int key;
    void * element;
    struct hash_node *next;
} hash_node;


typedef struct hash_t{
    int buckets;
    hash_node **head; //head list
    spinlock_t *hlock; //lock list
} hash_t;


void Hash_Init(hash_t *hash, int buckets);
void Hash_Insert(hash_t *hash, void *element, unsigned int key);
void Hash_Delete(hash_t *hash, unsigned int key);
void *Hash_Lookup(hash_t *hash, unsigned int key);

#endif
