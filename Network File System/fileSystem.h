#ifndef __FS_h__
#define __FS_h__
#include <stdio.h>
#include <stdlib.h>
#include "inodeBitmap.h"
#include "dataBitmap.h"
#include "inode.h"
#include "datablock.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct __FileSystem_t {
    INode_BitMap_t inodeBitMap;
    Data_BitMap_t dataBitMap;
    INode_t inodes[INODES_SIZE];
    Data_Block_t dataBlocks[DATA_BLOCKS_SIZE];
} FileSystem_t;

int init_file_system(char * image)
{
    FileSystem_t *fileSystem = (FileSystem_t *)malloc(sizeof(FileSystem_t));
    if(fileSystem == NULL){
        return -1;
    }

    // test if file exist, if exist, just return
    int fd = open(image, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd > -1)
    {
        free(fileSystem);
        close(fd);
        return 0;
    }
    // create one if not exist
    fd = open(image, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        free(fileSystem);
        close(fd);
        return -1;
    }

    // write to file
    ssize_t rc = write(fd, (void *)&fileSystem->inodeBitMap, sizeof(INode_BitMap_t));
    if (rc == -1)
    {
        free(fileSystem);
        close(fd);
        return -1;
    }
    fsync(fd);

    rc = write(fd, (void *)&fileSystem->dataBitMap, sizeof(Data_BitMap_t));
    if (rc == -1)
    {
        free(fileSystem);
        close(fd);
        return -1;
    }
    fsync(fd);
    rc = write(fd, (void *)fileSystem->inodes, sizeof(INode_t)*INODES_SIZE);
    if (rc == -1)
    {
        free(fileSystem);
        close(fd);
        return -1;
    }
    fsync(fd);
    rc = write(fd, (void *)fileSystem->dataBlocks, sizeof(Data_Block_t)*DATA_BLOCKS_SIZE);
    if (rc == -1)
    {
        free(fileSystem);
        close(fd);
        return -1;
    }
    fsync(fd);

    // add root directory
    // initialize empty inode
    INode_t inode;
    init_inode(&inode, MFS_DIRECTORY);
    int inum = allocate_inode(fd);

    // add root directory
    MFS_DirEnt_t entry;
    entry.inum = inum;
    strcpy(entry.name, ".");

    MFS_DirEnt_t entry2;
    entry2.inum = inum;
    strcpy(entry2.name, "..");

    int blockId = allocate_data_block(fd);
    inode.type = MFS_DIRECTORY;
    inode.dataBlocks[0] = blockId;
    inode.blocks++;
    inode.size += 2*DIRECTORY_ENTRY_SIZE;

    char* entries = get_data_block(fd, blockId);
    char* updatedEntries = insert_directory_entry(entries, &entry, 0);
    char* updatedEntries2 = insert_directory_entry(updatedEntries, &entry2, 1);
    
    char* tmp = updatedEntries2;
    MFS_DirEnt_t empty_entry[NUM_DIRECTORY_ENTRY-2];
    for(int i=0;i<NUM_DIRECTORY_ENTRY-2;i++){
        empty_entry[i].inum = -1;
        char* new_entries = insert_directory_entry(tmp, &empty_entry[i], i+2);
        memcpy(tmp,new_entries,sizeof(Data_Block_t));
        free(new_entries);
    }

    update_data_block(fd, blockId, tmp);
    update_inode(fd, inum, &inode);

    // free malloc
    free(fileSystem);
    free(entries);
    free(updatedEntries);
    free(updatedEntries2);
    close(fd);

    return 0;
}

#endif  // __FS_h__
