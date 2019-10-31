#ifndef counter_h
#define counter_h

#include "spin.h"

typedef struct counter_t {
    int local[20];		//local count
    spinlock_t llock[20];	//lock
}counter_t;

void Counter_Init(counter_t *c, int value);
int Counter_GetValue(counter_t *c);
void Counter_Increment(counter_t *c);
void Counter_Decrement(counter_t *c);


#endif
