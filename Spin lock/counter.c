#include "counter.h"
#include <sys/syscall.h>
#include <unistd.h>

void Counter_Init(counter_t *c, int value){
	int i;
	for(i=0; i<20; i++){
		c->local[i]=0;
		c->llock[i].flag = 0;
	}
	c->local[0] = value;
}

void Counter_Increment(counter_t *c) {
	int tid = syscall(SYS_gettid);
	int cpu = tid % 20;
	spinlock_acquire(&c->llock[cpu]);
	c->local[cpu]++;
	spinlock_release(&c->llock[cpu]);
}

void Counter_Decrement(counter_t *c) {
	int tid = syscall(SYS_gettid);
	int cpu = tid % 20;
	spinlock_acquire(&c->llock[cpu]);
	c->local[cpu]--;
	spinlock_release(&c->llock[cpu]);
}

int Counter_GetValue(counter_t *c) {
	int i;
	int total = 0;
	for(i=0; i<20; i++){
		spinlock_acquire(&c->llock[i]);
		total += c->local[i];
		spinlock_release(&c->llock[i]);
	}
	return total;
}
