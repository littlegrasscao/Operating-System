#ifndef list_h
#define list_h

#include "spin.h"

// basic node structure
typedef struct node_t {
    unsigned int key;
    void * element;
    struct node_t *next;
} node_t;

// basic list structure
typedef struct list_t {
    node_t *head;
    spinlock_t lock;
} list_t;


void List_Init(list_t *list);
void List_Insert(list_t *list, void *element, unsigned int key);
void List_Delete(list_t *list, unsigned int key);
void *List_Lookup(list_t *list, unsigned int key);

#endif
