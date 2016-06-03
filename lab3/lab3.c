    #include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <fcntl.h>

char command[64], input[128], str[128];
int i = 0;
int fileType=0;
FILE *fp;
char *fileName=NULL;

void ioredir();
void execute(char **myargv,char **envp);

int main(int argc, char** argv, char **envp)
{
    int pid;
    int status;
    while(1)
    {
	
	char args[64][64];
        char **myargv=args;
        printf("@hsuSh: " );
        fgets(input,128,stdin);
        strcpy(str,input);
	
        int oper;
        //printf("strstr: %d\n",strstr(input,"<"));
	if(memchr(input,'<',128)!=NULL)
        {
            fileType=1;
            fileName=memchr(input,'<',128);
	    fileName++;
            fileName[strlen(fileName)-1] = '\0';

        }
        if(memchr(input,'>',128)!=NULL)
        {
            fileType=2;
            fileName=memchr(input,'>',128);
            fileName++;
            fileName[strlen(fileName)-1] = '\0';


        }
        if(strstr(input,">>")!=NULL)
        {
            fileType=3;
            fileName=strstr(input,">>");
            fileName++;
            fileName[strlen(fileName)-1] = '\0';
        }
        char *token = strtok(str, " \n");
        if(token!=NULL)
        {
            strcpy(command, token);
        }

        if(strncmp(command, "exit",64)==0||strncmp(command, "q",64)==0)
        {
            exit(EXIT_SUCCESS);
        }

	i=0;
        while(token!=NULL)
        {
            if(strcmp(token,"<")==0||strcmp(token,">")==0||strcmp(token,">>")==0)
                break;
            //printf("Token: %s\n",token);
            myargv[i] = token;
            i++;
            token = strtok(NULL, " \n");
        }
	
	if(strncmp(command, "cd",64)==0)
    	{
       	 if(myargv[1]==NULL)
       	 {
		    printf("argument1: %s", myargv[1]);
        	    chdir(myargv[1]);
         	   
       	 }
      	 else
      	  {
            if(chdir(myargv[1])==-1)
                printf("chdir error\n");
            printf("CWD:%s\n",get_current_dir_name());
          }
	
    	}


	
        
        

        
        else{
		myargv[i]=NULL;
		pid =fork();
		if(pid){
			printf("Parent hsush PROC %d forks a child process %d\n", getpid(), pid);
			
			pid=wait(&status);
        		printf("Child %d dies, Status=%04x\n", pid, status);
			}
		else{
			execute(myargv,envp);
			exit(100);
		}
		
        
	}
    }// end while

}//end main

void execute(char **myargv,char **envp)
{
    
    char path[32][32] = {"/usr/local/sbin/ ","/usr/local/bin/",
                                "/usr/sbin/","/usr/bin/","/sbin/",
                                "/usr/games/","/usr/local/games","/bin/"};
    //int pid;
    //int status;
    //char *envp[] = { NULL };
    
    
     if(fileType==1)
		    {
		        close(0);
			open(fileName+1, O_RDONLY);
			fileType = 0;
		    } 

     if(fileType==2)
		    {
		        close(1);
		        open(fileName+1, O_WRONLY|O_CREAT, 0644);
		        fileType=0;
		    }
     if(fileType==3)
		    {
		        close(1);
		        //printf("fileName: %s",fileName);
		        open(fileName+2, O_RDWR|O_APPEND, 0644);
		        fileType=0;
		    }  
		
	
     while(execve(myargv[0],myargv,envp)){
		
		
		//printf("PROC %d Search in path: %s\n", getpid(), path[i]);
		myargv[0] = strcat(path[i],command);
		//exit(status);
		i++;
		if(i>8)
		{
		printf("Invalid command\n");
		return;
		}
		printf("PROC %d Search in path: %s\n", getpid(), path[i]);
	
	
	     

	}	
    
    return;		
	
    
}
