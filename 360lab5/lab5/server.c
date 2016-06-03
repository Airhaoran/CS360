// This is the echo SERVER server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>

#define  MAX 256

// Define variables:
struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int cusPort;
int  sock, newsock;                  // socket descriptors
int  serverPort;                     // server port number
int  r, length, n;                   // help variables

int fd;
int pid = 0, status = 0;
int fp;
char cwd[MAX];//current working directory
char hd[MAX];//homedirectory

int getRequest(int myargc, char* myargv[]);
int putRequest(int myargc, char* myargv[]);
int excute (int argc, char* argv[]);
char *getHomeDir(char *env[]);
int ls_dir(char *dname);
int ls_file(char *fname);

// enum state_name{
//   CLIENT_CONECT,
//   CLIENT_LOGIN,
//   CLIENT_PASSWORD
// }server_state; 
// Server initialization code:

int server_init(char *name, int number)
{
   printf("==================== server init ======================\n");   
   // get DOT name and IP address of this host

   printf("1 : get and show server host info\n");
   hp = gethostbyname(name);
   if (hp == 0){
      printf("unknown host\n");
      exit(1);
   }
   printf("    hostname=%s  IP=%s\n",
               hp->h_name,  inet_ntoa(*(long *)hp->h_addr));
  
   //  create a TCP socket by socket() syscall
   printf("2 : create a socket\n");
   sock = socket(AF_INET, SOCK_STREAM, 0);
   if (sock < 0){
      printf("socket call failed\n");
      exit(2);
   }

   printf("3 : fill server_addr with host IP and PORT# info\n");
   // initialize the server_addr structure
   server_addr.sin_family = AF_INET;                  // for TCP/IP
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // THIS HOST IP address  
   server_addr.sin_port = htons(number);   // let kernel assign port

   printf("4 : bind socket to host info\n");
   // bind syscall: bind the socket to server_addr info
   r = bind(sock,(struct sockaddr *)&server_addr, sizeof(server_addr));
   if (r < 0){
       printf("bind failed\n");
       exit(3);
   }

   printf("5 : find out Kernel assigned PORT# and show it\n");
   // find out socket port number (assigned by kernel)
   length = sizeof(name_addr);
   r = getsockname(sock, (struct sockaddr *)&name_addr, &length);
   if (r < 0){
      printf("get socketname error\n");
      exit(4);
   }

   // show port number
   serverPort = ntohs(name_addr.sin_port);   // convert to host ushort
   printf("    Port=%d\n", serverPort);

   // listen at port with a max. queue of 5 (waiting clients) 
   printf("5 : server is listening ....\n");
   listen(sock, 5);
   printf("===================== init done =======================\n");
}


main(int argc, char *argv[])
{
   //strcat(sucess, "Sucess");
   //strcat(failed, "Failed");
   char *hostname;
   char line[MAX];
   char temp[MAX];
   char *token;
   char* cmd;
   //char *myargv[32];
   int myargc;
   int i;
   bzero(cwd, MAX);
   hp = getenv("HOME");
   //strcpy(hd,getHomeDir(env));
   //uint16_t network_bytes_order_r, network_bytes_order_s;
   if (argc < 2)
      hostname = "localhost";
   else
      hostname = argv[1];
   if (argc < 3)
      cusPort = 3000;
   else
      cusPort = atoi(argv[2]);

   //cusPort = argv[2]; 
   server_init(hostname, cusPort); 
   //send(newsock, Welcom,MAX,MAX);
   // Try to accept a client request
   while(1){
     //int myargc = 0 ;
     char *myargv[32];
     int o = 0;
     printf("server: accepting new connection ....\n"); 

     // Try to accept a client connection as descriptor newsock
     length = sizeof(client_addr);
     newsock = accept(sock, (struct sockaddr *)&client_addr, &length);
     if (newsock < 0){
        printf("server: accept error\n");
        exit(1);
     }
     printf("server: accepted a client connection from\n");
     printf("-----------------------------------------------\n");
     printf("        IP=%s  port=%d\n", inet_ntoa(client_addr.sin_addr.s_addr),
                                        ntohs(client_addr.sin_port));
     printf("-----------------------------------------------\n");
     
     // Processing loop
     while(1){
       myargc = 0;
       n = read(newsock, line, MAX);
       if (n==0){
           printf("server: client died, server loops\n");
           close(newsock);
           break;
      }
      
      // show the line string
      printf("server: read  n=%d bytes; line=[%s]\n", n, line);
      printf("line + %s\n",line );
      strcpy(temp, line);
      token = strtok(temp," ");
      while(token != NULL){
        myargc++;
        myargv[o]=token;
        token = strtok(NULL, " ");
        o++;
      }
      
      cmd = myargv[0];
      excute(myargc, myargv);
      printf("\n");
      myargc = 0;
      o = 0;

      //bzero(myargv,MAX);
      // strcat(line, " ECHO");

      // // send the echo line to client 
      // n = write(newsock, line, MAX);

      // printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
      // printf("server: ready for next request\n");
    }
 }
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
  char ans[MAX];
  char sent[MAX];

  sp = &fstat;
  //printf("name=%s\n", fname); getchar();

  if ( (r = lstat(fname, &fstat)) < 0){
     printf("can't stat %s\n", fname); 
     //exit(1);
  }

  if ((sp->st_mode & 0xF000) == 0x8000){
       //printf("%c",'-');
     //strcat(ans,"-");
     sprintf(ans, "%c", '-');
     strcpy(sent, ans);
     //write(newsock,ans,MAX); 
   }
  if ((sp->st_mode & 0xF000) == 0x4000){
     //printf("%c",'d');
     //strcat(ans,"d");
     sprintf(ans, "%c", 'd');
     strcpy(sent, ans);
     //write(newsock,ans,MAX); 
   }
  if ((sp->st_mode & 0xF000) == 0xA000){
    // printf("%c",'l');
     //strcat(ans,"l");
     sprintf(ans, "%c", 'l');
     strcpy(sent, ans);
     //write(newsock,ans,MAX); 
   }

  for (i=8; i >= 0; i--){
    if (sp->st_mode & (1 << i)){
      
      sprintf(ans,"%c",t1[i]); 
      strcat(sent, ans); 
      }  
    else{
      
      sprintf(ans,"%c",t2[i]);
      strcat(sent, ans); 
    }
  }

  // printf("%4d ",sp->st_nlink);
  
  sprintf(ans,"%4d ",sp->st_nlink);
  strcat(sent, ans); 
  // printf("%4d ",sp->st_gid); 

  sprintf(ans,"%4d ",sp->st_gid);
  strcat(sent, ans);   
  // printf("%4d ",sp->st_uid);
  
  sprintf(ans,"%4d ",sp->st_uid);
  strcat(sent, ans);   
  //printf("%8d \n",sp->st_size);
  
  sprintf(ans,"%8d ",sp->st_size);
  strcat(sent, ans);
  
  // print time
  bzero(ftime,MAX);
  strcat(ftime, ctime(&sp->st_ctime));
  ftime[strlen(ftime)-1] = 0;
  sprintf(ans,"%s  ",ftime);
  strcat(sent, ans);
  //strcpy(ans, ftime)
  //write(newsock,ans,MAX); 
  
  // print name
  printf("is_file: %s\n", basename(fname)); 
  //bzero(ans,MAX);
  sprintf(ans,"%s",basename(fname));
  strcat(sent, ans);
  write(newsock,sent,MAX); 
  //printf("sent = :%s\n",sent );

  // print -> linkname if it's a symbolic file
  // if ((sp->st_mode & 0xF000)== 0xA000){ // YOU FINISH THIS PART
  //   char *linkname;
  //   const char *pathname;
  //   pathname = dirname(linkname);
  //   r = readlink(fname, linkname, sp->st_size+1);
  //   if (r <0){
  //     printf("readlink failed\n");
  //   }
  //    // use readlink() SYSCALL to read the linkname
  //   printf(" -> %s", *linkname);
  // }
  //printf("\n");
  //write(newsock, "new",MAX);
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
  write(newsock,"ls complete",MAX); 

}

int getCmd(int myargc, char* myargv[])
{
  int m = 0;
  int i, r;
  char buf[1024];
  struct stat fstat, *sp;
  char ans[MAX];
  //char sent[MAX]
  bzero(buf, 1024);
  bzero(ans,MAX);
  
  sp = &fstat;


  
      if(myargc > 1)
      {
        if(lstat(myargv[1], &fstat) < 0)
        {
          write(newsock, "0", 2);
          printf("can't lstat file\n");
          return 1;
        }
        else
        {
          if((sp->st_mode & 0xF000) == 0x4000)
          {
            printf("not going to give a dir\n");
            write(newsock, "0", 2);
            return 0;
          }
        }
        // sprintf(ans,"%8d ",sp->st_size);
        // //strcat(sent, ans);
        // printf("ans ===== %s\n\n\n\n\n\n", ans);
        // write(newsock, ans, MAX);
        fd = open(myargv[1], O_RDONLY);


        if(fd < 0)
        {
          printf("file open fail\n");
          //write(newsock, "0", 2);
        }
          
        write(newsock, "1", MAX);
        sprintf(ans,"%8d ",sp->st_size);
        //strcat(sent, ans);
        //printf("ans ===== %s\n\n\n\n\n\n", ans);
        write(newsock, ans, MAX);
        printf("putting file:%s to client\n", myargv[1]); 
        while(m = read(fd, buf, MAX))
        {
          
          write(newsock, buf, m);


        }

       
        close(fd);
    
      }
      else
      {
        write(newsock, "0", 2);
    
  
      }
}

int putCmd(int myargc, char *myargv[])
{
  char good[2];
  int m = 0, i = 0;
  char buf[1024];
  bzero(buf, 1024);
  int size =0;
  char res[MAX];
  char ans[MAX];
  int count = 0;
  int n = 0;
  //printf("Hi\n");
  //tell client to goahead and start putting out
  write(newsock, "1", MAX);

  //m = read(newsock, good, 2);
  //m = read(newsock, good, MAX);

  //if(strcmp(good, "0") == 0){return 0;}
  fp = open(myargv[1], O_WRONLY | O_CREAT);//open file
  read(newsock,res,MAX);
  //printf("res = %s\n", res);
  size = atoi(res);
  printf("size = %d\n", size);
  while( count < size )
  {
      n = read(newsock,buf,MAX);
    
      count += n;
       
      write(fp, buf,n);

  }
  printf("Succesed\n");
  close(fp);
  printf("File closed\n");

}
int excute (int myargc, char* myargv[])//command will not b null when called
{
  char answer[MAX];
  struct stat fstat, *sp;
  char *cmd;
  char *op = 0;
  sp = &fstat;
  bzero(answer, MAX);
  cmd  = myargv[0];
  char pwd[MAX];
  //printf("Hi\n");
  if(myargc > 1)
  {
    op = myargv[1];
  }
  
  if(strcmp(cmd, "pwd") == 0)
  {
    write(newsock, "1", MAX);//sending sucess message
    getcwd(cwd, MAX);
    write(newsock, cwd, MAX);
    printf("client request pwd: %s\n", cwd);
    return 1;
  } 
  if(strcmp(cmd, "ls") == 0)
  {
    //printf("Hi\n")  ;
    printf("myargc = %d\n",myargc );
    printf("myargv[0] = %s\n", myargv[0]);
    printf("myargv[1] = %s\n", myargv[1]);
    write(newsock, "1", MAX);
    if((myargc > 1) && lstat(myargv[1], &fstat) < 0)
    { 
      write(newsock, "0", 2);
      printf("lstat %s failed\n", myargv[1]);
      
      return 0;
    }
    else if(myargc > 1)
    {
      if(S_ISDIR(sp->st_mode))
      {
        //ls_dir(myargv[1]);
        strcat(answer, "dir");        
        write(newsock, answer, MAX);
        ls_dir(myargv[1]);
        return 0;
      }else{
        
        strcat(answer, "file");
        write(newsock, answer, MAX);
        ls_file(myargv[1]);
        //write(newsock,"done",MAX);
        return 0;
      }
    }

    if(myargc == 1)
    {
      strcat(answer, "dir");
      write(newsock, answer, MAX);
      ls_dir("./");
      printf("ls current directory OK");
      return 0;
    }
    
    return 1;
  } 
  if(strcmp(cmd, "cd") == 0)
  {
    printf("myargc =%d\n",myargc );
    printf("op = %s\n",op );
    write(newsock, "1", MAX);//start message
    if(myargc>1){
      getcwd(pwd, MAX);
      strcat(pwd, "/");
      strcat(pwd,op);
      printf("myargv[1] = %s\n",myargv[1] );
      printf("pwd = %s\n", pwd);
      if(!(chdir(pwd) < 0)){
        printf("client request: cd OK\n");
        strcat(answer, "cd OK");
        write(newsock, answer, MAX);
        printf("cd OK\n");
      }
    }
    // printf("myargc =%d\n",myargc );
    // printf("op = %s\n",op );
    // write(newsock, "1", MAX);//start message
    // if(!(chdir(pwd) < 0))//cd sucessed
    // {
      
    //   printf("client request: cd OK\n");
    //   strcat(answer, "cd OK");
    //   write(newsock, answer, MAX);
    //   printf("cd OK\n");
    // }
    if(myargc == 1)
    {
      
      printf("cd to HOME dir\n");
      chdir("/home/haoran");
      strcat(answer, "cd to HOME dir");
      write(newsock, answer, MAX);
      printf("cd to HOME dir\n");
      return 0;
    }
    else
    {
      printf("client request: cd FAILED\n");
      strcat(answer, "cd FAILED");
      write(newsock, answer, MAX);
      printf("cd FAILED\n");
    }
    return 1;
  } 
  if(strcmp(cmd, "mkdir") == 0)
  {
    printf("op = %s\n",op );
    write(newsock, "1", MAX);
    if(mkdir(op, 0777) < 0)
    {
      printf("mkdir FAIL \n");
      strcat(answer, "mkdir FAIL");
      write(newsock, answer, MAX);

      
    }else
    {
      printf("mkdir %s OK\n", op);
      strcat(answer, "mkdir OK");
      write(newsock, answer, MAX);
      printf("mkdir OK\n");
    }
    return 1;
  } 
  if(strcmp(cmd, "rmdir") == 0)
  {
    write(newsock, "1", MAX);
    if(rmdir(op) < 0)
    {
      
      printf("rmdir FAIL \n");
      strcat(answer, "rmdir FAIL ");
      write(newsock, answer, MAX);
      
      
    }else
    {
      printf("rmdir %s OK\n", op);
      strcat(answer, "rmdir OK");
      write(newsock, answer, MAX);
      printf("rmdir OK\n");

      //
    }
    return 1;
  } 
  if(strcmp(cmd, "rm") == 0)
  {
    write(newsock, "1", MAX);
    if(unlink(op) < 0)
    {
      printf("rm FAIL");
      strcat(answer, "rm FAIL");
      write(newsock, answer, MAX);

    }
    else
    {
      printf("rm %s OK\n", op);
      strcat(answer, "rm OK");
      write(newsock, answer, MAX);
      printf("rm OK\n");
    }
    return 1;
  } 
  if(strcmp(cmd, "get") == 0)
  {
    getCmd(myargc, myargv);
    return 2;//need to work with server
  } 
  if(strcmp(cmd, "put") == 0)
  {
    putCmd(myargc, myargv);   
    return 2;
  } 
  if(strcmp(cmd, "cat") == 0)
  {
    write(newsock, "1", MAX);
    strcat(answer, "Only cat local file");
    write(newsock, answer, MAX);
    
  }
  if(strcmp(cmd, "quit") == 0)//should never get this
  {
    exit(1);
  }

  return 0;
}