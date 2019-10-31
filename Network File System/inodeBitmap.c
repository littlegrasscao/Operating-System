#include "inodeBitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include "superBlock.h"
int if_inode_exist(int fd, int inum)
{
    if (fd < 0 || inum < 0
        || inum >= INODES_SIZE) return 0;

    Super_Block_t superBlock;
    init_super_block(&superBlock);

    //get position to read inode bitmap
    off_t offset = superBlock.inodeBitMapOffset;
    lseek(fd, offset, SEEK_SET);

    //read inode into pointer
    INode_BitMap_t bmap;
    ssize_t rc = read(fd, (void *)&bmap, sizeof(INode_BitMap_t));
    if (rc == -1) return 0;
    return bmap.inodes[inum];
}

void free_inode(int fd, int inum)
{
    if (fd < 0 || inum < 0
        || inum >= INODES_SIZE) return;

    Super_Block_t superBlock;
    init_super_block(&superBlock);

    //get position to read inode bitmap
    off_t offset = superBlock.inodeBitMapOffset;
    lseek(fd, offset, SEEK_SET);

    //read map into pointer
    INode_BitMap_t bmap;
    ssize_t rc = read(fd, (void *)&bmap, sizeof(INode_BitMap_t));
    if (rc == -1) return ;
    bmap.inodes[inum] = 0;

    update_inode_bitmap(fd, &bmap);
}

void update_inode_bitmap(int fd, INode_BitMap_t* imap)
{
    if (fd < 0 || imap == NULL) return;

    Super_Block_t superBlock;
    init_super_block(&superBlock);

    //set position to write data bitmap
    off_t offset = superBlock.inodeBitMapOffset;
    lseek(fd, offset, SEEK_SET);

    ssize_t rc = write(fd, (void *)imap, sizeof(INode_BitMap_t));
    if (rc == -1) return;
    fsync(fd);
}

int allocate_inode(int fd)
{
    if (fd < 0) return -1;

    Super_Block_t superBlock;
    init_super_block(&superBlock);

    //get position to read inode bitmap
    off_t offset = superBlock.inodeBitMapOffset;
    lseek(fd, offset, SEEK_SET);

    //read inode into pointer
    INode_BitMap_t bmap;
    ssize_t rc = read(fd, (void *)&bmap, sizeof(INode_BitMap_t));
    if (rc == -1) return -1;
    for (int i = 0; i < INODES_SIZE; ++i)
    {
        if (bmap.inodes[i] == 0)
        {
            bmap.inodes[i] = 1;
            update_inode_bitmap(fd, &bmap);
            return i;
        }
    }

    // inodes are full
    return -1;
}