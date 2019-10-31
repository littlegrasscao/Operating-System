#ifndef __DATABLOCKS_h__
#define __DATABLOCKS_h__
#include <sys/types.h>
#include <unistd.h>
#include "mfs.h"

#define BLOCK_SIZE   (4096)
#define DIRECTORY_ENTRY_SIZE (256)
#define NUM_DIRECTORY_ENTRY (16)

typedef struct __Data_Block_t {
    char data[BLOCK_SIZE];
} Data_Block_t;

char* get_data_block(int fd, int block);

MFS_DirEnt_t* get_directory_entry(char* dataBlock, int index);

char* insert_directory_entry(char* dataBlock, MFS_DirEnt_t* entry, int index);

void update_data_block(int fd, int block, char* dataBlock);
#endif // __DATABLOCKS_h__