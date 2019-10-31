#include "server.h"
#include "udp.h"
#include "fileSystem.h"

#define BUFFER_SIZE 5000

int main(int argc, char ** argv){
    if(argc < 3)
    {
        printf("PLease enter: server [portnum] [file-system-image]");
        exit(1);
    }

    int portnum = atoi(argv[1]);		//port number
    char * file_system_image = argv[2]; //name of file system image

    int sd = UDP_Open(portnum); //port # 
    assert(sd > -1);

    //initialize a file image
    init_file_system(file_system_image);
    int fd = open(file_system_image, O_RDWR, S_IRUSR | S_IWUSR);

    while (1) {
        struct sockaddr_in s;
        char buffer[BUFFER_SIZE];
        int rc = UDP_Read(sd, &s, buffer, BUFFER_SIZE); //read message buffer from port sd

        if (rc > 0) {

            int i;		//for loop index
            int j = 0; // name index
            int k = 0;  // comma index
            int max_para = 0;
            char func_name[15];
            int inum; //inode number or parent inode number
            int block; //block number or type 
            char content[4097]; //name or buffer write into block
            int len = strlen(buffer);
            char parameter[4096];

            for(i = 0; i < len; i++){	

                if(buffer[i] == ','){
                    parameter[j] = '\0';
                    j = 0;
                    if(k == 0){
                        if(strcmp(parameter,"MFS_Lookup") == 0){
                            strcpy(func_name, "MFS_Lookup\0");
                            max_para = 2;
                        }
                        else if(strcmp(parameter,"MFS_Stat") == 0){
                            strcpy(func_name, "MFS_Stat\0");
                            max_para = 1;
                        }
                        else if(strcmp(parameter,"MFS_Write") == 0){
                            strcpy(func_name, "MFS_Write\0");
                            max_para = 3;
                        }
                        else if(strcmp(parameter,"MFS_Read") == 0){
                            strcpy(func_name, "MFS_Read\0");
                            max_para = 2;
                        }
                        else if(strcmp(parameter,"MFS_Creat") == 0){
                            strcpy(func_name, "MFS_Creat\0");
                            max_para = 3;
                        }
                        else if(strcmp(parameter,"MFS_Unlink") == 0){
                            strcpy(func_name, "MFS_Unlink\0");
                            max_para = 2;
                        }
                    }
                    else if(k == 1){
                        inum = atoi(parameter);
                    }
                    else if(k == 2){
                        block = atoi(parameter);
                    }
                        
                    k++;
                    if(k == max_para){
                        if(strcmp(func_name,"MFS_Stat") == 0){
                            inum = atoi(buffer+i+1);
                        } 
                        else if(strcmp(func_name,"MFS_Read") == 0){
                            block = atoi(buffer+i+1);
                        }
                        else{
                            strcpy(content, buffer+i+1);
                        }
                        break;
                    }
                    else if(k > max_para){
                        break;
                    }
                }
                else{
                    //add char to parameter
                    parameter[j] = buffer[i];
                    j++;	    		
                } 

            }

            //output from functions
            int out;

            //call look up function
            if(strcmp(func_name,"MFS_Lookup") == 0){
                out = Server_Lookup(fd, inum, content);

                char reply[5];
                sprintf(reply, "%d", out);

                rc = UDP_Write(sd, &s, reply, 5);

            }
            //call stat function
            else if(strcmp(func_name,"MFS_Stat") == 0){
                MFS_Stat_t m;
                out = Server_Stat(fd, inum, &m);

                if(out == -1){
                    rc = UDP_Write(sd, &s, "-1", 2);
                }
                else{
                    rc = UDP_Write(sd, &s, (char*)&m, sizeof(MFS_Stat_t));
                }
            }
            //call write function
            else if(strcmp(func_name,"MFS_Write") == 0){
                out = Server_Write(fd, inum, content, block);
                
                char reply[5];
                sprintf(reply, "%d", out);

                rc = UDP_Write(sd, &s, reply, 5);
            }
            //call read function
            else if(strcmp(func_name,"MFS_Read") == 0){
                char data[4096];

                out = Server_Read(fd, inum, data, block);

                if(out == -1){
                    rc = UDP_Write(sd, &s, "-1", 2);
                }
                else{
                    rc = UDP_Write(sd, &s, data, 4096);
                }
            }
            //cal create function
            else if(strcmp(func_name,"MFS_Creat") == 0){
                out = Server_Creat(fd, inum, block, content);

                char reply[5];
                sprintf(reply, "%d", out);

                rc = UDP_Write(sd, &s, reply, 5);
            }
            //call unlink function
            else if(strcmp(func_name,"MFS_Unlink") == 0){
                out = Server_Unlink(fd, inum, content);

                char reply[5];
                sprintf(reply, "%d", out);
                rc = UDP_Write(sd, &s, reply, 5);
            }
        }
    }

    return 0;
}
