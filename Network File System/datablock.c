#include "superBlock.h"
#include "datablock.h"
#include "dataBitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include<string.h>

char* get_data_block(int fd, int block)
{
    //check if data block exists
    if(if_data_exist(fd, block) == 0)
    {
        return NULL;
    }

    Super_Block_t superBlock;
    init_super_block(&superBlock);
    //get position to read data content
    off_t offset = superBlock.dataRegionOffset + block*sizeof(Data_Block_t);
    lseek(fd, offset, SEEK_SET);

    //read data into pointer
    char * data = (char*)malloc(sizeof(Data_Block_t));
    ssize_t rc = read(fd, (void *)data, sizeof(Data_Block_t));

    if (rc == -1)
    {
        free(data);
        return NULL;
    }

    return data;
}

MFS_DirEnt_t* get_directory_entry(char* dataBlock, int index)
{
    if (index < 0 || index >= NUM_DIRECTORY_ENTRY) return NULL;
    MFS_DirEnt_t *entry = (MFS_DirEnt_t*)malloc(sizeof(MFS_DirEnt_t));
    memcpy(entry, dataBlock + index * sizeof(MFS_DirEnt_t), sizeof(MFS_DirEnt_t));
    return entry;
}

char* insert_directory_entry(char* dataBlock, MFS_DirEnt_t* entry, int index)
{
    char *newData = (char*)malloc(sizeof(Data_Block_t));
    // copy old data
    memcpy(newData, dataBlock, sizeof(Data_Block_t));

    memcpy(newData + index * sizeof(MFS_DirEnt_t), entry, sizeof(MFS_DirEnt_t));

    return newData;
}

void update_data_block(int fd, int block, char* dataBlock)
{
    //check if data exists
    if (if_data_exist(fd, block) == 0)
    {
        return;
    }

    Super_Block_t superBlock;
    init_super_block(&superBlock);

    //get position to read data content
    off_t offset = superBlock.dataRegionOffset + block * sizeof(Data_Block_t);
    lseek(fd, offset, SEEK_SET);

    //write inode
    ssize_t rc = write(fd, (void *)dataBlock, sizeof(Data_Block_t));
    if (rc == -1)
    {
        return;
    } 
    fsync(fd);
}
