#include "mfs.h"
#include "udp.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 5000

fd_set rfds;
int sd = -1;
struct sockaddr_in addr;

int MFS_Init(char *hostname, int port)
{
    sd = UDP_Open(8000);
    
    int rc = UDP_FillSockAddr(&addr, hostname, port); //contact server at specified port

    if(rc == -1)
        return -1;
    else
        return 0;
}

int udp_socket(char * message, char * buffer){
   while(1){
        int rc = UDP_Write(sd, &addr, message, strlen(message)+1); //write message to server@specified-port

        //send successfully
        if (rc > 0) {
            //initialize fd
            FD_ZERO(&rfds);
            FD_SET(sd, &rfds);

            //set timeout
            struct timeval tv;
            tv.tv_sec = 5;
            tv.tv_usec = 0; 
            
            //select read timeout
            int ret = select(sd+1, &rfds, NULL, NULL, &tv);
            //check select failure
            if (ret < 0)
            {
                return -1;
            } 
            //if receive read replies
            else if(FD_ISSET(sd, &rfds)){
                UDP_Read(sd, &addr, buffer, BUFFER_SIZE); //read message from ...

                //if reply from server is an error
                if (buffer[0]=='-' && buffer[1]=='1'){
                    return -1;
                }

                break;
            }
        }
        //send message failure
        else{
            return -1;
        } 
    }
    //if success
    return 0;   
}

int MFS_Lookup(int pinum, char *name)
{
    assert(sd > -1);
    if(pinum < 0 || pinum >= 4096 || name == NULL){
        return -1;
    }

    //convert int to string
    char snum[5];
    sprintf(snum, "%d", pinum);

    //send message 
    char * message = malloc(sizeof(char)*300);
    sprintf(message, "MFS_Lookup,%s,%s", snum, name);

    char buffer[BUFFER_SIZE] = "-1";

    int ret = udp_socket(message, buffer);
    //free malloc space
    free(message);

    //error check
    if(ret == -1){
        return -1;
    }


    //convert into int
    int inum = atoi(buffer);
    return inum;
}

int MFS_Stat(int inum, MFS_Stat_t *m)
{
    assert(sd > -1);
    if(inum < 0 || inum >= 4096 || m == NULL){
        return -1;
    }

    //convert int to string
    char snum[5];
    sprintf(snum, "%d", inum);

    //send message 
    char * message = malloc(sizeof(char)*30);
    sprintf(message, "MFS_Stat,%s", snum);

    int ret = udp_socket(message, (char*)m);

    //free malloc space
    free(message);

    //error check
    if(ret == -1){
        return -1;
    }

    return 0;
}

int MFS_Write(int inum, char *buffer, int block)
{
    assert(sd > -1);

    if(inum >= 4096 || inum < 0 || block >= 4096 || block < 0
        || buffer == NULL){
        return -1;
    }

    //convert int to string
    char snum[5];
    sprintf(snum, "%d", inum);

    //convert int to string
    char sblock[5];
    sprintf(sblock, "%d", block);

    //buffer length
    //char buf2[MFS_BLOCK_SIZE+1];
    char* buf2 = malloc(sizeof(char)*(MFS_BLOCK_SIZE+1));
    memcpy(buf2,buffer,MFS_BLOCK_SIZE);
    buf2[4096] = '\0';

    //send message 
    char * message = malloc(sizeof(char)*BUFFER_SIZE);
    sprintf(message, "MFS_Write,%s,%s,%s", snum, sblock, buf2);
    char ret_buffer[] = "-1";

    int ret = udp_socket(message, ret_buffer);

    //free malloc space
    free(buf2);
    free(message);

    //error check
    if(ret == -1){
        return -1;
    }
    return 0;
}

int MFS_Read(int inum, char *buffer, int block)
{
    assert(sd > -1);
    if(inum < 0 || inum >= 4096 || block < 0 || block >= 4096){
        return -1;
    }

    //convert int to string
    char snum[5];
    sprintf(snum, "%d", inum);

    //convert int to string
    char sblock[5];
    sprintf(sblock, "%d", block);   

    //send message 
    char * message = malloc(sizeof(char)*50);
    sprintf(message, "MFS_Read,%s,%s", snum, sblock);

    int ret = udp_socket(message, buffer);

    //free malloc space
    free(message); 
    //error check
    if(ret == -1){
        return -1;
    }

    return 0;
}

int MFS_Creat(int pinum, int type, char *name)
{
    assert(sd > -1);
    if(pinum < 0 || pinum >= 4096){
        return -1;
    }

    //convert int to string
    char snum[5];
    sprintf(snum, "%d", pinum);

    //convert int to string
    char stype[5];
    sprintf(stype, "%d", type);

    //buffer length
    int len = strlen(name);
    //if buffer to big
    if(len > 252){
        return -1;
    }

    //send message 
    char * message = malloc(sizeof(char)*300);
    sprintf(message, "MFS_Creat,%s,%s,%s", snum, stype, name);

    char buffer[BUFFER_SIZE] = "-1";

    int ret = udp_socket(message, buffer);

    //free malloc space
    free(message);

    //error check
    if(ret == -1){
        return -1;
    }

    return 0;
}

int MFS_Unlink(int pinum, char *name)
{
    assert(sd > -1);
    if(pinum < 0 || pinum >= 4096){
        return -1;
    }

    //convert int to string
    char snum[5];
    sprintf(snum, "%d", pinum);

    //buffer length
    int len = strlen(name);
    //if buffer to big
    if(len > 252){
        return -1;
    }

    //send message 
    char * message = (char *)malloc(sizeof(char)*300);
    sprintf(message, "MFS_Unlink,%s,%s", snum, name);

    char buffer[BUFFER_SIZE] = "-1";

    int ret = udp_socket(message, buffer);

    //free malloc space
    free(message);

    //error check
    if(ret == -1){
        return -1;
    }

    return 0;
}
