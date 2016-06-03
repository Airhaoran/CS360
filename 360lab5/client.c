//**************************** ECHO CLIENT CODE **************************
// The echo client client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>/

#define MAX 256
#define MAX_LOGIN 20
#define REVBUFFSIZE 256

// Define variables
struct hostent *hp;              
struct sockaddr_in  server_addr; 

int sock, r;
int SERVER_IP, SERVER_PORT; 

int checkLocal (char *cmd);
int checkValid(char *cmd);
char* removeL(char *cmd);
int lCmd(int argc, char *argv[]);
char* getHomeDir(char *env[]);


int ls_file(char *fname);
int ls_dir(char *dname);


char cwd[MAX];//current working directory
char hd[MAX];//homedirectory
char res[MAX];

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";


int fd;
int fp;


int client_init(char *argv[])
{
  printf("======= clinet init ==========\n");

  printf("1 : get server info\n");
  hp = gethostbyname(argv[1]);
  if (hp==0){
     printf("unknown host %s\n", argv[1]);
     exit(1);
  }

  SERVER_IP   = *(long *)hp->h_addr;
  SERVER_PORT = atoi(argv[2]);

  printf("2 : create a TCP socket\n");
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock<0){
     printf("socket call failed\n");
     exit(2);
  }

  printf("3 : fill server_addr with server's IP and PORT#\n");
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = SERVER_IP;
  server_addr.sin_port = htons(SERVER_PORT);

  // Connect to server
  printf("4 : connecting to server ....\n");
  r = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (r < 0){
     printf("connect failed\n");
     exit(1);
  }

  printf("5 : connected OK to \007\n"); 
  printf("---------------------------------------------------------\n");
  printf("hostname=%s  IP=%s  PORT=%d\n", 
          hp->h_name, inet_ntoa(SERVER_IP), SERVER_PORT);
  printf("---------------------------------------------------------\n");

  printf("========= init done ==========\n");
}

main(int argc, char *argv[ ])
{
  int n, i, m;
  char line[MAX], ans[MAX], temp[MAX];
  hp = getenv("HOME");
  char *token;
  char *piece;
  char *cmd;
  char *myargv[32];
  char myargc;
  //int o = 0;


  if (argc < 3){
     printf("Usage : client ServerName SeverPort\n");
     exit(1);
  }

  client_init(argv);

  printf("********  processing loop  *********\n");
  while (1){
    int o =0;
    printf("input a line : ");
    bzero(line, MAX);                // zero out line[ ]
    fgets(line, MAX, stdin);         // get a line (end with \n) from stdin
    myargc = 0;
    line[strlen(line)-1] = 0;        // kill \n at end
    if (line[0]==0)                  // exit if NULL line
       exit(0);
    strcpy(temp, line);
    token = strtok(temp," ");
    while(token != NULL){
      myargc++;
      myargv[o]=token;
      token = strtok(NULL, " ");
      o++;
    }
    //printf("Hi\n");
    //printf("myargv0 :%s,myargv1: %s \n",myargv[0], myargv[1] );
    cmd = myargv[0];
    //printf("Hi\n");
    if(strcmp(cmd, "quit") == 0)
    {
      exit(0);
    }
    if(!checkLocal(cmd) && checkValid(cmd)){
      n = write(sock, line, MAX);
      printf("client: wrote n=%d bytes; line=(%s)\n", n, line);
      m = read(sock, res, MAX);
      if(strcmp(res, "1") == 0)//sucess
      {
        printf("server responsed:\n");
        response(cmd, myargc, myargv);

      }
      else if(strcmp(res, "0") == 0)//fail
      {
        printf("No response from server.\n");
    
      }
   
    //printf("Hi\n");

    }
    else if(checkLocal(cmd) && checkValid(removeL(cmd))){
      //printf("Hi\n");
      cmd = removeL(cmd);
      printf("Local command :%s\n", cmd);
      lCmd(myargc, myargv);
    }
    myargc = 0;
    if (checkValid(cmd) ==0)
      printf("Bad command\n");
    //bzero(myargv,0);
    // n = write(sock, line, MAX);
    // printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

    // // Read a line from sock and show it
    // n = read(sock, ans, MAX);
    // printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
  }
    }

int checkLocal(char *cmd)
{
  int i = 0;

  if(strcmp(cmd, "ls") == 0)//if it's ls, it's not local
  {
    return 0;
  }

  //l is the first
  if(cmd[0] == 'l')
  {
    return 1; 
  }

  //l is not first
  return 0;
}

int checkValid (char* cmd)
{
  if(cmd == 0)
  {
    return 0;
  }

  if(strcmp(cmd, "pwd") == 0)
  {
    return 1;
  } if(strcmp(cmd, "ls") == 0)
  {
    return 1;
  } if(strcmp(cmd, "cd") == 0)
  {
    return 1;
  } if(strcmp(cmd, "mkdir") == 0)
  {
    return 1;
  } if(strcmp(cmd, "rmdir") == 0)
  {
    return 1;
  } if(strcmp(cmd, "rm") == 0)
  {
    return 1;
  } if(strcmp(cmd, "get") == 0)
  {
    return 1;
  } if(strcmp(cmd, "put") == 0)
  {
    return 1;
  } 
    
  if(strcmp(cmd, "cat") == 0)
  {
    return 1;
  }
  if(strcmp(cmd, "quit") == 0)
  {
    return 1;
  }

  return 0;
}

char* removeL(char *cmd)//remove the first char (used to remove 'l')
{
  int i = 1;
  char* noL = 0;//(char*)malloc(sizeof(char));

  if(strlen(cmd) <=  1)//if one or 0 chars (or less) return 0
  {
    return noL;
  }

  while(cmd[i])
  {
    noL = (char *) realloc (noL, (i+1) * (sizeof(char)));
    noL[i - 1] = cmd[i];
    noL[i++] = 0;
  }

  return noL;
}
int lCmd (int myargc, char* myargv[])//command will not b null when called
{

  //function will only be called for local commands
  printf("Hi\n");
  char *cmd = myargv[0];
  struct stat fstat, *sp;
  char *op = 0;
  int i, n;
  char buf[1024];
  printf("Hi\n");
  cmd = removeL(cmd);
  sp = &fstat;
  char pwd[MAX];

  if(myargc > 1)
  {
    
    op = myargv[1];
  }
  
  if(strcmp(cmd, "pwd") == 0)
  {
    getcwd(cwd, MAX);
    printf("lpwd: %s\n", cwd);
    return 1;
  } 
  if(strcmp(cmd, "ls") == 0)
  {
    //printf("Hi\n");
    printf("myargc = %d\n",myargc);
    if((myargc > 1) && lstat(myargv[1], &fstat) < 0)
    { 
      printf("lstat %s\n failed", myargv[1]);
      return 0;
    }
    else if(myargc > 1)
    {
      if(S_ISDIR(sp->st_mode))
      {
        //printf("Hi\n");
        ls_dir(myargv[1]);
        return 0;
      }else{
        //printf("Hi\n");
        ls_file(myargv[1]);
        return 0;
      }
    }

    if(myargc == 1)
    {
      printf("ls current dir\n");
      ls_dir("./");
      return 0;
    }

    //printf("wtf happen with lls\n");
    //need to ls all files
    
    return 1;
  } 
  if(strcmp(cmd, "cd") == 0)
  {
    if(myargc>1){
      getcwd(pwd, MAX);
      strcat(pwd, "/");
      strcat(pwd,op);
      printf("myargv[1] = %s\n",myargv[1] );
      printf("pwd = %s\n", pwd);
      if(!(chdir(pwd) < 0))
        printf("lcd OK\n");     
    }
    // if(!(chdir(pwd) < 0))//if argv available and chdir success
    // {
    //   printf("lcd OK\n");     

    // }
    else if(myargc == 1)
    {
      printf("lcd to HOME dir\n");
      chdir("/home/haoran"); 
      return 0;     
    }
    else
    {
      printf("lcd FAIL\n");
    }
    return 1;
  } 
  if(strcmp(cmd, "mkdir") == 0)
  {
    if(mkdir(op, 0777) < 0)
    {
      printf("lmkdir FAIL -> errno=%d : %s\n", errno, strerror(errno));
    }else
    {
      printf("lmkdir %s OK\n", op);
    }
    return 1;
  } 
  if(strcmp(cmd, "rmdir") == 0)
  {
    if(rmdir(op) < 0)
    {
      printf("lrmkdir FAIL -> errno=%d : %s\n", errno, strerror(errno));
    }else
    {
      printf("lrmkdir %s OK\n", op);
    }
    return 1;
  } 
  if(strcmp(cmd, "rm") == 0)
  {
    if(unlink(op) < 0)
    {
      printf("lrm FAIL -> errno=%d : %s\n", errno, strerror(errno));
    }
    else
    {
      printf("lrm %s OK\n", op);
    }
    return 1;
  } 
  if(strcmp(cmd, "get") == 0)
  {
    printf("use (get) instead.\n");   
    return 2;//need to work with server
  } 
  if(strcmp(cmd, "put") == 0)
  {
    printf("use (put) instead.\n");   
    return 2;
  }
  if(strcmp(cmd, "cat")==0)
  {
    if(myargc < 2)
    {
      printf("Nothing to be cat\n");
      return 1;
    }

    if(lstat(myargv[1], &fstat) < 0)
    {
      printf("lstat %s\n failed", myargv[1]);
      return 0;
    }else
    {
      if(!S_ISREG(sp->st_mode))
      {
        printf("Only stat regular file\n");
        return 0;
      }
    }
    
    fd = open(myargv[1], O_RDONLY);

    if(fd < 0)
    {
      printf("cat failed\n");
      return 1;
    }

    while(n = read(fd, buf, 1024)){
      for(i = 0; i < n; i++)
      {
        putchar(buf[i]);
      }
    }
  
    close(fd);
  }
  if(strcmp(cmd, "quit") == 0)
  {
    exit(1);
  }

  return 0;
}
char *getHomeDir(char *env[])
{
  int i = 0;
  char line[MAX];
  char *hd = 0;

  //assuming HOME path exists
  while(env[i])
  {
    strcpy(line, env[i]);
    hd = strtok(line, "=");
    
    if(strcmp(hd, "HOME") == 0)
    {
      //assuming HOME=something
      hd = strtok(NULL, "=");
      break;
    }
    i++;
  }

  return hd;
}

int ls_file(char *fname)
{
  struct stat fstat, *sp;
  int r, i;
  char ftime[64];


  sp = &fstat;
  //printf("name=%s\n", fname); getchar();

  if ( (r = lstat(fname, &fstat)) < 0){
     printf("can't stat %s\n", fname); 
     exit(1);
  }

  if ((sp->st_mode & 0xF000) == 0x8000)
     printf("%c",'-');
  if ((sp->st_mode & 0xF000) == 0x4000)
     printf("%c",'d');
  if ((sp->st_mode & 0xF000) == 0xA000)
     printf("%c",'l');

  for (i=8; i >= 0; i--){
    if (sp->st_mode & (1 << i))
  printf("%c", t1[i]);
    else
  printf("%c", t2[i]);
  }

  printf("%4d ",sp->st_nlink);
  printf("%4d ",sp->st_gid);
  printf("%4d ",sp->st_uid);
  printf("%8d ",sp->st_size);

  // print time
  strcpy(ftime, ctime(&sp->st_ctime));
  ftime[strlen(ftime)-1] = 0;
  printf("%s  ",ftime);

  // print name
  printf("%s", basename(fname));  

  // print -> linkname if it's a symbolic file
  if ((sp->st_mode & 0xF000)== 0xA000){ // YOU FINISH THIS PART
    char *linkname;
    const char *pathname;
    pathname = dirname(linkname);
    r = readlink(fname, linkname, sp->st_size+1);
    if (r <0){
      printf("readlink failed\n");
    }
     // use readlink() SYSCALL to read the linkname
    printf(" -> %s", *linkname);
  }
  printf("\n");
}

int ls_dir(char *dname)
{
  DIR *dp;
  struct dirent *ep;
  dp = opendir(dname);
  while ( ep=readdir(dp)){
  ls_file(ep->d_name);
  //printf("%s \n", ep->d_name);
  } 

}

int response(char* cmd, int myargc, char *myargv[])
{
  int n = 0, i = 0;
  int m= 0;
  char buf[1024];
  char response[MAX];
  char sent[MAX];
  char done[2];
  bzero(buf, 1024);
  char res[MAX];
  int count = 0;
  int size =0;
  char ans[MAX];
  struct stat fstat, *sp;
  sp = &fstat;
  //printf("Im in response@!!!!!\n");
  if(myargc > 1 && strcmp(cmd, "get") == 0)
  {
 
    //printf("Im in get@!!!!!\n");
    fp = open(myargv[1], O_WRONLY | O_CREAT);
    read(sock,res,MAX);
    //printf("res = %s\n", res);
    size = atoi(res);
    printf("size = %d\n", size);
    while( count < size)
    {
       n = read(sock,buf,MAX);
    
       count += n;
       
       write(fp, buf,n);

    }
    printf("done - server served you!\n");
    close(fp);
    chmod(myargv[1],0755);
    printf("File closed\n");
  }
  else if(strcmp(cmd, "put") == 0)
  {
    if(myargc > 1)
      {
        if(lstat(myargv[1], &fstat) < 0)
        {
          write(sock, "0", 2);
          printf("can't lstat file\n");
          return 1;
        }
        else
        {
          if((sp->st_mode & 0xF000) == 0x4000)
          {
            printf("not going to give a dir\n");
            write(sock, "0", 2);
            return 0;
          }
        }

        fd = open(myargv[1], O_RDONLY);


        if(fd < 0)
        {
          printf("file open fail\n");
          //write(newsock, "0", 2);
        }
          
        //write(sock, "1", MAX);
        sprintf(ans,"%8d ",sp->st_size);
        //strcat(sent, ans);
        //printf("ans ===== %s\n\n\n\n\n\n", ans);
        write(sock, ans, MAX);
        printf("putting file:%s to client\n", myargv[1]); 
        while(m = read(fd, buf, MAX))
        {
          
          write(sock, buf, m);


        }

       
        close(fd);
    
      }
      else
      {
        write(sock, "0", 2);
    
  
      }
      //return 2;//need to write more to server
  }
  else if(strcmp(cmd, "pwd") == 0)
  {
    n = read(sock, cwd, MAX);
    printf("server gives pwd: %s\n", cwd);
  }else if(strcmp(cmd, "cd") == 0)
  {
    n = read(sock, response, MAX);
    printf("cd server response: %s\n", response);
  }else if(strcmp(cmd, "mkdir") == 0)
  {
    n = read(sock, response, MAX);
    printf("mkdir server response: %s\n", response);
  }else if(strcmp(cmd, "rmdir") == 0)
  {
    n = read(sock, response, MAX);
    printf("rmdir server response: %s\n", response);
  }else if(strcmp(cmd, "rm") == 0)
  {
    n = read(sock, response, MAX);
    printf("rm server response: %s\n", response);
  }else if(strcmp(cmd, "cat") == 0){

    n = read(sock, response, MAX);
    printf("server response: %s\n", response);  
  }else if(strcmp(cmd, "ls") == 0){
    n = read(sock, response, MAX);
    printf("server response: %s\n", response); 
    if (strcmp(response,"dir")==0){
      //n = read(sock, response, MAX);
      
      while (strcmp(response, "ls complete")!= 0){
    
        bzero(response,MAX);
        read(sock, response,MAX);
     

        printf("%s\n",response);
      
      }
      
    }
    else //(strcmp(response, "file")==0){
      {
      bzero(response,MAX);
      read(sock, response,MAX);
      printf("%s\n",response );
      return 2;
      }
      
    }
    //break;
  }
  //break;

  
  

