#include "superBlock.h"
#include "inode.h"
#include "inodeBitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include "mfs.h"

void init_inode(INode_t *inode, int type)
{
    inode->blocks = 0;
    inode->size = 0;
    inode->type = type;
    for (int i = 0; i < POINTER_SIZE; ++i)
    {
        inode->dataBlocks[i] = -1;
    }
}

INode_t* get_inode(int fd, int inodeNumber)
{
    //check if inode exists
    if(if_inode_exist(fd, inodeNumber) == 0)
    {
        return NULL;
    }

    Super_Block_t superBlock;
    init_super_block(&superBlock);

    //get position to read inode content
    off_t offset = superBlock.inodeRegionOffset + inodeNumber*sizeof(INode_t);
    lseek(fd, offset, SEEK_SET);

    //read inode into pointer
    INode_t* inode = (INode_t*)malloc(sizeof(INode_t));
    ssize_t rc = read(fd, (void *)inode, sizeof(INode_t));
    if (rc == -1)
    {
        free(inode);
        return NULL;
    }
    return inode;
}

void update_inode(int fd, int inodeNumber, INode_t *inode)
{
    //check if inode exists
    if (if_inode_exist(fd, inodeNumber) == 0)
    {
        return;
    }

    Super_Block_t superBlock;
    init_super_block(&superBlock);

    //get position to read inode content
    off_t offset = superBlock.inodeRegionOffset + inodeNumber * sizeof(INode_t);
    lseek(fd, offset, SEEK_SET);

    //write inode
    ssize_t rc = write(fd, (void *)inode, sizeof(INode_t));
    if (rc == -1) return;
    fsync(fd);
}