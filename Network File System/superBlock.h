#ifndef __SUPER_BLOCK_h__
#define __SUPER_BLOCK_h__
#include <sys/types.h>
#include <unistd.h>
typedef struct __Super_Block_t {
    off_t inodeBitMapOffset;
    off_t dataBitMapOffset;
    off_t inodeRegionOffset;
    off_t dataRegionOffset;
} Super_Block_t;

void init_super_block(Super_Block_t *superBlock);

#endif // __SUPER_BLOCK_h__
