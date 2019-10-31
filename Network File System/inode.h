#ifndef __INODE_h__
#define __INODE_h__
#include <sys/types.h>
#include <unistd.h>

#define POINTER_SIZE   (10)

typedef struct __INode_t {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
    int blocks; // number of blocks allocated to file
    int dataBlocks[POINTER_SIZE];
} INode_t;

void init_inode(INode_t *inode, int type);

INode_t* get_inode(int fd, int inodeNumber);

void update_inode(int fd, int inodeNumber, INode_t *inode);

#endif // __INODE_h__
