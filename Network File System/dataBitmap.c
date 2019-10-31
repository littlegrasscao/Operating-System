#include "dataBitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include "superBlock.h"

int if_data_exist(int fd, int block)
{
    if (fd < 0 || block < 0
        || block >= DATA_BLOCKS_SIZE) return 0;

    Super_Block_t superBlock;
    init_super_block(&superBlock);

    //get position to read data bitmap
    off_t offset = superBlock.dataBitMapOffset;
    lseek(fd, offset, SEEK_SET);

    //read data into pointer
    Data_BitMap_t bmap;
    ssize_t rc = read(fd, (void *)&bmap, sizeof(Data_BitMap_t));
    if (rc == -1) return 0;

    return bmap.dataBlocks[block];
}

void free_data_block(int fd, int block)
{
    if (fd < 0 || block < 0
        || block >= DATA_BLOCKS_SIZE) return;

    Super_Block_t superBlock;
    init_super_block(&superBlock);

    //get position to read data bitmap
    off_t offset = superBlock.dataBitMapOffset;
    lseek(fd, offset, SEEK_SET);

    //read inode into pointer
    Data_BitMap_t bmap;
    ssize_t rc = read(fd, (void *)&bmap, sizeof(Data_BitMap_t));
    if (rc == -1) return;

    bmap.dataBlocks[block] = 0;

    update_data_bitmap(fd, &bmap);
}

void update_data_bitmap(int fd, Data_BitMap_t* bmap)
{
    if (fd < 0 || bmap == NULL) return;

    Super_Block_t superBlock;
    init_super_block(&superBlock);

    //set position to write data bitmap
    off_t offset = superBlock.dataBitMapOffset;
    lseek(fd, offset, SEEK_SET);

    ssize_t rc = write(fd, (void *)bmap, sizeof(Data_BitMap_t));
    if (rc == -1) return;
    fsync(fd);
}

int allocate_data_block(int fd)
{
    if (fd < 0) return -1;

    Super_Block_t superBlock;
    init_super_block(&superBlock);

    //get position to read data bitmap
    off_t offset = superBlock.dataBitMapOffset;
    lseek(fd, offset, SEEK_SET);

    //read inode into pointer
    Data_BitMap_t bmap;
    ssize_t rc = read(fd, (void *)&bmap, sizeof(Data_BitMap_t));
    if (rc == -1) return -1;

    for (int i = 0; i < DATA_BLOCKS_SIZE; ++i)
    {
        if (bmap.dataBlocks[i] == 0)
        {
            bmap.dataBlocks[i] = 1;
            update_data_bitmap(fd, &bmap);
            return i;
        }
    }

    // data block is full
    return -1;


}