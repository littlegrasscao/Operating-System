#include "list.h"
#include <stdio.h>
#include <stdlib.h>

//initlized head and lock
void List_Init(list_t *list){
    list->head = NULL;
    list->lock.flag = 0;
}

void List_Insert(list_t *list, void *element, unsigned int key){
	//malloc a new node
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    new_node->key = key;
    new_node->element = element;

    //critical section
    spinlock_acquire(&list->lock);
    new_node->next = list->head;
    list->head = new_node;
    spinlock_release(&list->lock);
}

void List_Delete(list_t *list, unsigned int key){
    spinlock_acquire(&list->lock);
    //get head node
    node_t *curr = list->head;
    //if empty
    if(!curr){}
    //if need to delete head
    else if(curr->key == key){
        list->head = curr->next;
        free(curr);
    }
    //if need to delete other keys
    else{
        while(curr->next){
            if(curr->next->key == key){
                node_t *tmp = curr->next;
                curr->next = tmp->next;
                free(tmp);
                break;
            }
            curr = curr->next;
        }
    }
    spinlock_release(&list->lock);    
}

void *List_Lookup(list_t *list, unsigned int key){
    spinlock_acquire(&list->lock);
    //search from the head node
    node_t *curr = list->head;
    while(curr){
        if(curr->key == key){
            spinlock_release(&list->lock);
            return curr->element;
        }
        curr = curr->next;
    }
    spinlock_release(&list->lock);
    return NULL;
}
