#include "hash.h"
#include <stdlib.h>

void Hash_Init(hash_t *hash, int buckets){
	//malloc space
	hash->buckets = buckets;
	hash->head = (hash_node**)malloc(sizeof(hash_node*)*buckets);
    hash->hlock = (spinlock_t*)malloc(sizeof(spinlock_t)*buckets);

    //initialize to 0
    int i;
    for(i=0; i<buckets; i++){
        hash->head[i]=NULL;
        hash->hlock[i].flag = 0;
    }
}

void Hash_Insert(hash_t *hash, void *element, unsigned int key){
	//malloc a new node
    hash_node *new_node = (hash_node *)malloc(sizeof(hash_node));
    new_node->key = key;
    new_node->element = element;

	//select a bucket
    int bucket = key % hash->buckets;
    //insert critical section
    spinlock_acquire(&hash->hlock[bucket]);
    new_node->next = hash->head[bucket];
    hash->head[bucket] = new_node;
    spinlock_release(&hash->hlock[bucket]);
}

void Hash_Delete(hash_t *hash, unsigned int key){
    //select a bucket
    int i = key % hash->buckets;
    //delete node
    spinlock_acquire(&hash->hlock[i]);
    //get head node
    hash_node *curr = hash->head[i];
    //if need to delete head
    if(curr && curr->key == key){
        hash->head[i] = curr->next;
        free(curr);
    }
    //if need to delete other keys
    else{
        while(curr && curr->next){
            if(curr->next->key == key){
                hash_node *tmp = curr->next;
                curr->next = tmp->next;
                free(tmp);
                break;
            }
            curr = curr->next;
        }
    }
    spinlock_release(&hash->hlock[i]); 
}


void *Hash_Lookup(hash_t *hash, unsigned int key){
	//select a bucket
	int i = key % hash->buckets;
    //find a node - critical section
    spinlock_acquire(&hash->hlock[i]);
    hash_node * curr = hash->head[i];
    while(curr){
        if(curr->key == key){
            spinlock_release(&hash->hlock[i]);
            return curr->element;
        }
        curr = curr->next;
    }
    spinlock_release(&hash->hlock[i]);
    return NULL;
}

