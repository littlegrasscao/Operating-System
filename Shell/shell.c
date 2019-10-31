#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define MAX_INPUT 256 
#define PATH_MAX 1024

//error message
void Error(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

//split a command into arguments
int split(char** args, char* command, char* split_symbol){
    char *arg;
    int i=0;
    //split all the arguments by split_symbol
    arg = strtok(command, split_symbol);
    while(arg){
        args[i] = arg;
        i++;
        arg = strtok(NULL, split_symbol);
    }
    args[i]=NULL;    

    return i;
}

//badic shell process, fork new processes
int Basic_Shell(char **args, int separate, int parallel, char * f){
    pid_t pid;
    if ((pid = fork()) < 0){
        perror(""); //SYSTEM ERROR
        return 1;
    }
    //child process
    else if (pid == 0){
        // redireciton
        if (f){
            close(1);
            open(f,O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
        }
        //execute commands
        if(execvp(args[0],args) < 0){
            perror(""); //system error
        }        
        exit(0);
    }
    //parent process
    else{
        if(separate){
            int stat;    
            //while(waitpid(pid, &stat, 0) == 0){sleep(1)};
            while(1){
                pid_t end = waitpid(pid, &stat, 0);
                if(end != 0){break;}
            }
        }
        else if(!separate && !parallel){wait(NULL);}
        return 1;
    }
}

//support pipelines
void Piplines(char ***command) 
{
  int   p[2];
  pid_t pid;
  int   in = 0;

  //executes all input commands 
  while (*command != NULL)
    {
      pipe(p); //pip
      if ((pid = fork()) == -1){
          perror(""); //SYSTEM ERROR
        }
      else if (pid == 0)
        {
          dup2(in, 0); //change the input
          if (*(command + 1) != NULL)
            dup2(p[1], 1);
          close(p[0]);
          //execute commands
          if(execvp((*command)[0], *command) < 0){
              perror(""); //system error
          }
          exit(0);
        }
      else
        {
          wait(NULL);
          close(p[1]);
          in = p[0]; //save the input
          command++;
        }
    }
}


//parse commands and execute Shell process
void Shell_Process(char *command, int * stat){    
    //multiple commands
    int parallel = 0;
    int separate = 0;
    for(int n=0; n<MAX_INPUT ;n++){
        if(command[n]){
            //find ; in command
            if(command[n]==';'){
                separate++;}
            //find + in command
            if(command[n]=='+'){
                parallel++;}
        }
        else{break;}         
    }                
    //error
    if(parallel && separate){
        Error();
    }
    else{
    //malloc space and separate input
    char **args = (char**)malloc(MAX_INPUT*sizeof(char*));
    int i = split(args, command, "\r\n;+");

    //run individual command arguments
    for(int a=0;a<i;a++){
        //redirection and pipeline
        int redirec = 0;
        int pip = 0;
        int err = 0;
        for(int n=0; n<MAX_INPUT ;n++){
            //check double || or >>
            if(args[a][n] && args[a][n+1]){
                if(args[a][n]=='|' && args[a][n+1]=='|'){
                    err = 1;
                    break;
                }
            }
            //count > and |
            if(args[a][n]){
                //find > in command
                if(args[a][n]=='>'){
                    redirec++;}
                //find | in command
                else if(args[a][n]=='|'){
                    pip++;}
            }
            else{break;}         
        }               

        //error if || or >>
        if(err){
            Error();
        }
        //error if both occur
        else if(redirec && pip){
            Error();
        //if more than 1 symbols
        }else if(redirec>1){
            Error();
        }
        //redirection
        else if(redirec){
            //split commands by >
            char** split_args = (char**)malloc(MAX_INPUT*sizeof(char*));
            split(split_args, args[a], ">");

            //right side arguments of >
            char** r_args = (char**)malloc(MAX_INPUT*sizeof(char*));
            int k = split(r_args, split_args[1], "\t ");  
                                
            //left side arguments of >
            char** l_args = (char**)malloc(MAX_INPUT*sizeof(char*));
            split(l_args, split_args[0], "\t ");
            
            if(k != 1){
                Error();}
            else{
                *stat = Basic_Shell(l_args, separate, parallel, r_args[0]);
            }
            free(l_args);
            free(r_args);
            free(split_args);
        }   
        //deal with pipeline
        else if(pip){
            //split commands by |
            char** split_args = (char**)malloc(MAX_INPUT*sizeof(char*));
            int x = split(split_args, args[a], "|");
            
            //split each command to arguments
            char** cmd[x+1];
            for (int n=0; n< x; n++){
                cmd[n] = (char**)malloc(MAX_INPUT*sizeof(char*));
                split(cmd[n], split_args[n], "\t\"\' "); //split by space and quote
            }
            cmd[x]=NULL;   

            //run piplines
            Piplines(cmd);

            //free space
            for(int n=0; n< x; n++){
                free(cmd[n]);
            }
            free(split_args);
        }else{
        	//copy args[a]
        	char * args_copy = (char*)malloc((strlen(args[a])+1)*sizeof(char));
        	strcpy(args_copy,args[a]);

            //split by space
            char** indi_args = (char**)malloc(MAX_INPUT*sizeof(char*));
            split(indi_args, args[a], "\t ");

            //build-in commands
            //quit
            if(strcmp(indi_args[0],"quit") == 0){
                *stat = 0;
            }
		    //cd
		    else if(strcmp(indi_args[0],"cd") == 0){
                //home directory
		        if (indi_args[1] == NULL){
		            chdir(getenv("HOME"));
		        }
                //only allow one argument
                else if(indi_args[2]){
                    Error();
                }
                //cd dir
		        else{
		            if(chdir(indi_args[1]) != 0){
		                Error();
		            }
		        }
		    }
            //echo
            else if(strcmp(indi_args[0],"echo") == 0){
            	//check quote
            	int quote = 0; 
            	char string[MAX_INPUT];
            	int index = 0; 
            	for(int n=0; n < strlen(args_copy); n++){
		            //count " amd '
		            if(args_copy[n]){
		                //find  in command
		                if((args_copy[n]=='\"' || args_copy[n]=='\'') && !quote){
		                    quote = 1;}
		                //if quote print the first string
		                else if((args_copy[n]=='\"' || args_copy[n]=='\'') && quote){
		                	write(STDOUT_FILENO, string, index);
		                	break;
		                }
		                //copy char
		                else if(quote){
		                	string[index] = args_copy[n];
		                	index++;
		                }
		            }     
		        }
		        //print one word
		        if(!quote){
                	write(STDOUT_FILENO, indi_args[1], strlen(indi_args[1]));
		        }   
		        write(STDOUT_FILENO, "\n", 1);    
            }
            else{
                //error if pwd follows arguments
                if(strcmp(indi_args[0],"pwd") == 0 && indi_args[1]){
                    Error();
                }
                else{
                    //update status by running commands
                    *stat = Basic_Shell(indi_args, separate, parallel, NULL);
                }
            }
            //free memory
            free(indi_args);
            free(args_copy);    
        }
        //if enter quit    
        if(*stat == 0){
            break;
        } 
    }
    
    //wait all processes done
    if(parallel){
        pid_t wpid;
        int status = 0;
        while ((wpid = wait(&status)) > 0); 
    }
    //free memory
    free(args);
    }
}


//command loop
void Interactive_Shell(){
    int stat = 1;

    while(stat){
        //print prompt
        printf("520sh> ");

        //read from stdin
        char *command = (char*)malloc(sizeof(char)*(MAX_INPUT+2));
        
        if(fgets(command, MAX_INPUT+2, stdin)){
            //if length of input command is larger than 256
            if(strlen(command) > MAX_INPUT){
                Error();
                //deal with rest of line
                int ch;
                while ((ch = fgetc(stdin)) != '\n') {}
            }
            else{   
                Shell_Process(command, &stat);
            }
        }
        free(command);
    }
}

//batch mode
void Batch_Shell(char * filename){
    //open file
    FILE * f;
    f = fopen(filename,"r");
    //fail to open a file
    if (f == NULL){
        Error();
        exit(1);
    }

    //varibales
    int stat = 1;
    char * cmdline = (char*)malloc(sizeof(char)*(MAX_INPUT+2));
   
    //read every line
    while(fgets(cmdline, MAX_INPUT+2, f) != NULL){
        //write command to stdout
        int length = strlen(cmdline);
        write(STDOUT_FILENO, cmdline, length);

        //if length of input command is larger than 256
        if(strlen(cmdline) > MAX_INPUT){
            //deal with rest of line
            char* ch = malloc(sizeof(char));
            while ((*ch = fgetc(f)) != '\n') {write(STDOUT_FILENO, ch, 1);}
            write(STDOUT_FILENO, "\n", 1);
            free(ch);
            //error message
            Error();
        }
        else{  
            Shell_Process(cmdline,&stat);
        }
    }

    //close and free
    fclose(f);
    free(cmdline);
}

int main(int argc, char **argv)
{
    //batch mode
    if(argc==2){
        Batch_Shell(argv[1]);
    }
    //Interactive mode
    else if (argc == 1){
        Interactive_Shell();
    }
    //error
    else{
        Error();
        return 1;
    }
    
  return 0;
}


