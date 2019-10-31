#ifndef spin_h
#define spin_h

// struct spinlock_t;

struct spinlock_t
{
    unsigned int flag;
};

typedef struct spinlock_t spinlock_t;

void spinlock_acquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock);


#endif  
