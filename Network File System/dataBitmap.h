#ifndef __DATA_BMAP_h__
#define __DATA_BMAP_h__
#define DATA_BLOCKS_SIZE   (4096)
#include <sys/types.h>
#include <unistd.h>
typedef struct __Data_BitMap_t {
    int dataBlocks[DATA_BLOCKS_SIZE];
} Data_BitMap_t;

int if_data_exist(int fd, int block);

void free_data_block(int fd, int block);

void update_data_bitmap(int fd, Data_BitMap_t* bmap);

int allocate_data_block(int fd);

#endif // __DATA_BMAP_h__
