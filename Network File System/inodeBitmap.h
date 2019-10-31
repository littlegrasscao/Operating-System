#ifndef __INODE_BMAP_h__
#define __INODE_BMAP_h__
#include <sys/types.h>
#include <unistd.h>

#define INODES_SIZE   (4096)

typedef struct __INode_BitMap_t {
    int inodes[INODES_SIZE];
} INode_BitMap_t;

int if_inode_exist(int fd, int inum);

void free_inode(int fd, int inum);

void update_inode_bitmap(int fd, INode_BitMap_t* imap);

int allocate_inode(int fd);
#endif // __INODE_BMAP_h__
