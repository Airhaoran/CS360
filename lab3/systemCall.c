#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>

struct stat mystat, *sp;

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

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
  // char path[1000];
  //   strcpy(path,dname);
  //   DIR *dp;
  //   struct dirent *files;
  //   /*structure for storing inode numbers and files in dir
  //   struct dirent
  //   {
  //       ino_t d_ino;
  //       char d_name[NAME_MAX+1]
  //   }
  //   */
  //   if((dp=opendir(path))==NULL)
  //       perror("dir\n");
  //   char newp[1000];
  //   struct stat buf;
  //   while((files=readdir(dp))!=NULL)
  //   {
  //   if(!strcmp(files->d_name,".") || !strcmp(files->d_name,".."))
  //   continue;

  //       //strcpy(newp,path);
  //       //strcat(newp,"/");
  //       strcpy(newp,files->d_name); 
  //       //printf("%s\n",newp);
    
  //       ls_file(newp);

  //           //stat function return a structure of information about the file    
  //       if(stat(newp,&buf)==-1)
  //         perror("stat");

  //   }
  // struct stat fstat, *sp;
  // int r, i;
  // char ftime[64];

  // sp = &fstat;
  // //printf("name=%s\n", fname); getchar();

  // if ( (r = lstat(dname, &fstat)) < 0){
  //    printf("can't stat %s\n", dname); 
  //    exit(1);
  // }

  // if ((sp->st_mode & 0xF000) == 0x8000)
  //    printf("%c",'-');
  // if ((sp->st_mode & 0xF000) == 0x4000)
  //    printf("%c",'d');
  // if ((sp->st_mode & 0xF000) == 0xA000)
  //    printf("%c",'l');

  // for (i=8; i >= 0; i--){
  //   if (sp->st_mode & (1 << i))
  // printf("%c", t1[i]);
  //   else
  // printf("%c", t2[i]);
  // }

  // printf("%4d ",sp->st_nlink);
  // printf("%4d ",sp->st_gid);
  // printf("%4d ",sp->st_uid);
  // printf("%8d ",sp->st_size);

  // // print time
  // strcpy(ftime, ctime(&sp->st_ctime));
  // ftime[strlen(ftime)-1] = 0;
  // printf("%s  ",ftime);

  // // print name
  // printf("%s", basename(dname));  

  // // print -> linkname if it's a symbolic file
  // if ((sp->st_mode & 0xF000)== 0xA000){ // YOU FINISH THIS PART
  //     DIR *dp;
  //     struct dirent *ep;
  //     dp = opendir(dname);
  //     while ( ep=readdir(dp)){
  //        printf("%s ", ep->d_name);
  //    } 
  // }
  // printf("\n");
}

main(int argc, char *argv[])
{
  struct stat mystat, *sp;
  int r;
  char *s;
  char name[1024], cwd[1024];
  int a, b;

  s = argv[1];
  if (argc==1)
     s = "./";

  sp = &mystat;
  if (r = lstat(s, sp) < 0){
     printf("no such file %s\n", s); exit(1);
  }
  strcpy(name, s);
  if (s[0] != '/'){    // name is relative : get CWD path
     getcwd(cwd, 1024);
     strcpy(name, cwd); strcat(name, "/"); strcat(name,s);
  }
  if (S_ISDIR(sp->st_mode)){
      a= ls_dir(name);
      printf("just want to know what ls_dir will return: %d\n", a);
    }
  else{
      b = ls_file(name);
      printf("just want to know what ls_file will return: %d\n", b);
    }

}