#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <fcntl.h>

char command[64], input[128], str[128];
int i = 0;
int filestyle=0;
FILE *fp;
char *iopos=NULL;

void ioredir();
void execute(char **myargv,char **envp);

int main(int argc, char** argv, char **envp)
{
    while(1)
    {
        printf("@MySh :" );
        fgets(input,128,stdin);
        strcpy(str,input);
        int oper;
        //printf("strstr: %d\n",strstr(input,"<"));
	printf("this is input: %s\nthis is str: %s\n",input,str);
        if(memchr(input,'<',128)!=NULL)
        {
            filestyle=1;
            iopos=memchr(input,'<',128);
            iopos[strlen(iopos)-1] = '\0';

        }
        if(memchr(input,'>',128)!=NULL)
        {
            filestyle=2;
            iopos=memchr(input,'>',128);
            iopos++;
            iopos[strlen(iopos)-1] = '\0';


        }
        if(strstr(input,">>")!=NULL)
        {
            filestyle=3;
            iopos=strstr(input,">>");
            iopos++;
            iopos[strlen(iopos)-1] = '\0';
        }
        //printf("filestyle:%x\n",filestyle);
        char *token = strtok(str, " \n");
        if(token!=NULL)
        {
            strcpy(command, token);
        }

        if(strncmp(command, "exit",64)==0||strncmp(command, "q",64)==0)
        {
            exit(EXIT_SUCCESS);
        }

        char args[64][64];
        char **myargv=args;
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
        myargv[i]=NULL;
        execute(myargv,envp);
    }// end while

}//end main

void execute(char **myargv,char **envp)
{
    
	
    if(strncmp(command, "cd",64)==0)
    {
        if(myargv[1]==NULL)
        {
            chdir(envp[5]+5);
	    
            //printf("CWD:%s\n",get_current_dir_name());
        }
	
        else
        {
            if(chdir(myargv[1])==-1)
                printf("chdir error\n");
            printf("CWD:%s\n",get_current_dir_name());
	    
        }
	return;
    }
    
    char path[32][32] = {"/usr/local/sbin/ ","/usr/local/bin/",
                                "/usr/sbin/","/usr/bin/","/sbin/",
                                "/usr/games/","/usr/local/games","/bin/"};
    int pid;
    pid = fork();
    printf("Parent hsush PROC %d forks a child process %d\n", getpid(), pid);	
    int status=1;
    //char *envp[] = { NULL };
    for(i=0;i<10&&status!=0;i++)
    {
	if(i <8){
	  
		myargv[0] = strcat(path[i],command);
		printf("Search in path: %s\n", path[i]);
		switch (pid)
		{
		
		case -1:
		    perror("fork()");
		    exit(EXIT_FAILURE);
		    break;	
		case 0:
		    if(filestyle==1)
		    {
		        filestyle=0;
		    } else
		    if(filestyle==2)
		    {
		        close(1);
		        open(iopos+1, O_WRONLY|O_CREAT, 0644);
		        filestyle=0;
		    }
		    if(filestyle==3)
		    {
		        close(1);
		        printf("IOPOS: %s",iopos);
		        open(iopos+2, O_RDWR|O_APPEND, 0644);
		        filestyle=0;
		    }
		    //printf("Parent hsush PROC %d forks a child process %d\n", getppid(), getpid());	
		    status = execve(myargv[0],myargv,envp);
 		    printf("status= %d\n", status);
		    exit(status);
		    break;
		default:
		    if( waitpid(pid, &status, 0) < 0)
		    {
		        perror("waitpid()");
		        exit(EXIT_FAILURE);
		    }
		    if( WIFEXITED(status))
		    {
		        if(status != 65280)
		            printf("child exit status code: %d\n",status);
		        break; // do not exit get next input
		    }
		    exit(EXIT_FAILURE); // should not ever get here
		}
		
		}
		
	if(i > 8){
		printf("Invalid command\n");
		}
	
	
    }
}
