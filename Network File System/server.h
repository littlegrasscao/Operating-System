#ifndef __SERVER_h__
#define __SERVER_h__
#include "mfs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "inode.h"
#include "mfs.h"
#include "datablock.h"
#include "dataBitmap.h"
#include <string.h>
#include "inodeBitmap.h"
int Server_Lookup(int fd, int pinum, char *name)
{
    // check input
    if (pinum < 0 || pinum >= INODES_SIZE
        || name == NULL || fd < 0) return -1;

    // find directory inode
    INode_t* directory_inode = get_inode(fd, pinum);

    // if not found, error. invalid pinum
    if (directory_inode == NULL) return -1;

    // if not a directory, error.
    if (directory_inode->type != MFS_DIRECTORY)
    {
        free(directory_inode);
        return -1;
    }

    for (int i = 0; i < POINTER_SIZE; ++i)
    {
        int block = directory_inode->dataBlocks[i];
        // must be a valid block id
        if (block != -1)
        {
            char* data = get_data_block(fd, block);
            // block must be in use
            if (data == NULL)
            {
                free(directory_inode);
                return -1;
            }

            for (int j = 0; j < NUM_DIRECTORY_ENTRY; ++j)
            {
                MFS_DirEnt_t* entry = get_directory_entry(data, j);
                // directory entry must be valid
                if (entry == NULL)
                {
                    free(entry);
                    free(data);
                    free(directory_inode);
                    return -1;
                }

                // looks up the entry name in it in the entry is in use
                if (entry->inum != -1 && strcmp(name, entry->name) == 0)
                {
                    int ret = entry->inum;
                    free(entry);
                    free(data);
                    free(directory_inode);

                    return ret;
                }
                free(entry);
            }
            free(data);
        }
    }
    // name does not exist in pinum
    free(directory_inode);
    return -1;
}

int Server_Stat(int fd, int inum, MFS_Stat_t *m)
{
    // check input
    if (inum < 0 || inum >= INODES_SIZE || m == NULL || fd < 0) return -1;

    // find inode
    INode_t* inode = get_inode(fd, inum);

    // if not found, error. invalid pinum
    if (inode == NULL) return -1;

    m->blocks = inode->blocks;
    m->size = inode->size;
    m->type = inode->type;
    free(inode);
    return 0;
}

int Server_Write(int fd, int inum, char *buffer, int block)
{

    // check input
    if (inum < 0 || inum >= INODES_SIZE || buffer == NULL || fd < 0
        || block < 0 || block >= POINTER_SIZE) return -1;

    // find file inode
    INode_t* file_inode = get_inode(fd, inum);

    // if not found, error. invalid inum
    if (file_inode == NULL) return -1;

    // if not a file, error.
    if (file_inode->type != MFS_REGULAR_FILE)
    {
        free(file_inode);
        return -1;
    }
    // start write to the file
    // check if block is already allocated, create one if not allocated
    if (file_inode->dataBlocks[block] == -1)
    {
        int blockId = allocate_data_block(fd);

        // no free data block
        if (blockId == -1)
        {
            free(file_inode);
            return -1;
        }
        file_inode->dataBlocks[block] = blockId;
        file_inode->blocks++;
        file_inode->size += BLOCK_SIZE;
    }

    update_data_block(fd, file_inode->dataBlocks[block], buffer);
    update_inode(fd, inum, file_inode);

    return 0;
}

int Server_Read(int fd, int inum, char *buffer, int block)
{
    // check input
    if (inum < 0 || inum >= INODES_SIZE || buffer == NULL || fd < 0
        || block < 0 || block >= POINTER_SIZE) return -1;

    // find inode
    INode_t* inode = get_inode(fd, inum);

    // if not found, error. invalid inum
    if (inode == NULL) return -1;

    int blockId = inode->dataBlocks[block];
    // must be a valid block id
    if (blockId == -1)
    {
        free(inode);
        return -1;
    }

    char* data = get_data_block(fd, blockId);
    // block must be in use
    if (data == NULL)
    {
        free(inode);
        return -1;
    }

    // copy data to buffer
    memcpy(buffer, data, BLOCK_SIZE);
    free(inode);
    free(data);

    return 0;
}

int Server_Creat(int fd, int pinum, int type, char *name)
{
    // check input
    if (pinum < 0 || pinum >= INODES_SIZE || name == NULL || fd < 0) return -1;

    // find directory inode
    INode_t* directory_inode = get_inode(fd, pinum);

    // if not found, error. invalid pinum
    if (directory_inode == NULL) return -1;

    // if not a directory, error.
    if (directory_inode->type != MFS_DIRECTORY)
    {
        free(directory_inode);
        return -1;
    }
    // confirm name is not exist, otherwise do nothing. return success.
    int ret = Server_Lookup(fd, pinum, name);
    if(ret != -1)
    {
        free(directory_inode);
        return ret;
    }
    // create a file or a directory
    if (type == MFS_REGULAR_FILE || type == MFS_DIRECTORY)
    {
        // add record to parent directory if there is still available entry space
        // find an empty entry in a block
        int found = 0;
        int blockNum = 0;
        int entryNum = 0;
        for (int i = 0; i < POINTER_SIZE; ++i)
        {
            int block = directory_inode->dataBlocks[i];
            // must be a valid block id
            if (block != -1) {
                char* data = get_data_block(fd, block);
                for (int j = 0; j < NUM_DIRECTORY_ENTRY; ++j)
                {
                    MFS_DirEnt_t* entry = get_directory_entry(data, j);
                    if (entry != NULL)
                    {
                        // looks up the inum
                        if (entry->inum == -1)
                        {
                            blockNum = i;
                            entryNum = j;
                            found = 1;
                            free(entry);
                            free(data);
                            goto out;
                        }
                    }
                }
                free(data);
            }
        }

     out:
        // allocated new block if existing full.
        if (found == 0) {
            blockNum = -1;
            for (int i = 0; i < POINTER_SIZE; ++i)
            {
                int block = directory_inode->dataBlocks[i];
                // must be a valid block id
                if (block == -1) {
                    blockNum = i;
                    entryNum = 0;
                    break;
                }
            }
            // no empty block. Whole directory is full.
            if (blockNum == -1)
            {
                free(directory_inode);
                return -1;
            }
            // allocate new block
            else
            {
                int data = allocate_data_block(fd);
                // data if full
                if (data == -1)
                {
                    free(directory_inode);
                    return -1;
                }
                directory_inode->dataBlocks[blockNum] = data;
                directory_inode->blocks++;
            }
        }
        // allocate new inode and data
        int inum = allocate_inode(fd);
        // inode is full.
        if (inum == -1)
        {
            free(directory_inode);
            return -1;
        }

        // initialize empty inode
        INode_t inode;
        init_inode(&inode, type);
        // if this is a directory, need to add parent and current dir to directory entries.
        if (type == MFS_DIRECTORY)
        {
            int blockId = allocate_data_block(fd);
            // block is full.
            if (blockId == -1)
            {
                free(directory_inode);
                return -1;
            }
            inode.type = MFS_DIRECTORY;
            inode.dataBlocks[0] = blockId;
            inode.blocks++;
            // will be 2 entries
            inode.size += DIRECTORY_ENTRY_SIZE * 2;
            char* entries = get_data_block(fd, blockId);

            // current
            MFS_DirEnt_t current;
            current.inum = inum;
            strcpy(current.name, ".");
            char* updatedEntries = insert_directory_entry(entries, &current, 0);
            free(entries);

            // parent
            MFS_DirEnt_t parent;
            parent.inum = pinum;
            strcpy(parent.name, "..");
            char* updatedEntries2 = insert_directory_entry(updatedEntries, &parent, 1);
            free(updatedEntries);

            char* tmp = updatedEntries2;
            MFS_DirEnt_t empty_entry[NUM_DIRECTORY_ENTRY-2];
            for(int i=0;i<NUM_DIRECTORY_ENTRY-2;i++){
                empty_entry[i].inum = -1;
                char* new_entries = insert_directory_entry(tmp, &empty_entry[i], i+2);
                memcpy(tmp,new_entries,sizeof(Data_Block_t));
                free(new_entries);
            }

            update_data_block(fd, blockId, tmp);
            free(updatedEntries2); 
        }
        update_inode(fd, inum, &inode);

        // add this entry to parent directory
        MFS_DirEnt_t entry;
        entry.inum = inum;
        strcpy(entry.name, name);

        directory_inode->size += DIRECTORY_ENTRY_SIZE;
        char* entries = get_data_block(fd, directory_inode->dataBlocks[blockNum]);
        char* updatedEntries = insert_directory_entry(entries, &entry, entryNum);
        update_data_block(fd, directory_inode->dataBlocks[blockNum], updatedEntries);
        update_inode(fd, pinum, directory_inode);

        free(entries);
        free(updatedEntries);
        free(directory_inode);
    }
    // not a directoy or file, error
    else
    {
        free(directory_inode);
        return -1;
    }
    return 0;
}

int Server_Unlink(int fd, int pinum, char *name)
{
    // check input
    if (pinum < 0 || pinum >= INODES_SIZE || name == NULL || fd < 0) return -1;

    // find directory inode
    INode_t* directory_inode = get_inode(fd, pinum);

    // if not found, error. invalid pinum
    if (directory_inode == NULL) return -1;

    // if not a directory, error.
    if (directory_inode->type != MFS_DIRECTORY)
    {
        free(directory_inode);
        return -1;
    }

    for (int i = 0; i < POINTER_SIZE; ++i)
    {
        int block = directory_inode->dataBlocks[i];
        // must be a valid block id
        if (block != -1)
        {
            char* data = get_data_block(fd, block);
            // block must be in use
            if (data != NULL)
            {
                for (int j = 0; j < NUM_DIRECTORY_ENTRY; ++j)
                {
                    MFS_DirEnt_t* entry = get_directory_entry(data, j);
                    if (entry != NULL)
                    {
                        // looks up the entry name in it
                        if (strcmp(name, entry->name) == 0 && entry->inum != -1)
                        {
                            // find inode
                            INode_t* entry_inode = get_inode(fd, entry->inum);
                            // if not found, error. invalid inum
                            if (entry_inode == NULL)
                            {
                                free(data);
                                free(entry);
                                free(directory_inode);
                                return -1;
                            }
                            // removes the directory
                            if (entry_inode->type == MFS_DIRECTORY)
                            {
                                if (entry_inode->size <= 2*DIRECTORY_ENTRY_SIZE)
                                {
                                    // free all data block
                                    for (int k = 0; k < POINTER_SIZE; ++k) {
                                        free_data_block(fd, entry_inode->dataBlocks[k]);
                                    }

                                    // free inode
                                    free_inode(fd, entry->inum);

                                    // remove entry from parent directory
                                    entry->inum = -1;
                                    directory_inode->size -= DIRECTORY_ENTRY_SIZE;

                                    // update info in file.
                                    char* updatedData = insert_directory_entry(data, entry, j);
                                    update_data_block(fd, block, updatedData);
                                    update_inode(fd, pinum, directory_inode);

                                    free(data);
                                    free(directory_inode);
                                    free(entry_inode);
                                    free(entry);
                                    return 0;
                                }
                                else {
                                    //  the to-be-unlinked directory is NOT empty
                                    free(data);
                                    free(directory_inode);
                                    free(entry_inode);
                                    free(entry);
                                    return -1;
                                }
                            }
                            // removes the file
                            else {
                                // free all data block
                                for (int k = 0; k < POINTER_SIZE; ++k) {
                                    free_data_block(fd, entry_inode->dataBlocks[k]);
                                }

                                 // free inode
                                free_inode(fd, entry->inum);

                                // remove entry from parent directory
                                entry->inum = -1;
                                directory_inode->size -= DIRECTORY_ENTRY_SIZE;

                                // update info in file.
                                char* updatedData = insert_directory_entry(data, entry, j);
                                update_data_block(fd, block, updatedData);
                                update_inode(fd, pinum, directory_inode);

                                free(data);
                                free(directory_inode);
                                free(entry_inode);
                                free(entry);
                                return 0;
                            }
                            free(entry_inode);
                        }
                    }
                    free(entry);
                }
                free(data);
            }
        }
    }
    free(directory_inode);
    // the name not existing is NOT a failure
    return 0;
}

#endif