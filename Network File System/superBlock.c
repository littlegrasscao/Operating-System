#include "superBlock.h"
#include "dataBitmap.h"
#include "inodeBitmap.h"
#include "inode.h"
void init_super_block(Super_Block_t *superBlock)
{
    superBlock->inodeBitMapOffset = 0;
    superBlock->dataBitMapOffset = superBlock->inodeBitMapOffset + sizeof(INode_BitMap_t);
    superBlock->inodeRegionOffset = superBlock->dataBitMapOffset + sizeof(Data_BitMap_t);
    superBlock->dataRegionOffset = superBlock->inodeRegionOffset + sizeof(INode_t) * INODES_SIZE;
}