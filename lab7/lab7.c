#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

// Block number of EXT2 FS on FD


// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

typedef struct OFT{
  int   mode;
  int   refCount;
  struct Minode *minodeptr;
  int   offset;
}OFT;

// PROC structure
typedef struct proc{
  int   uid;
  int   pid, gid;
  int   status;
  struct Minode *cwd;
  OFT   *fd[10];
} PROC;
      
// In-memory inodes structure
typedef struct Minode{    
  INODE INODE;               // disk inode
  int   dev, ino;

  int   refCount;
  int   dirty;
  int   mounted;
  struct Mount *mountptr;
  char     name[128];           // name string of file
} MINODE;

// Mount Table structure
typedef struct Mount{
        int    ninodes;
        int    nblocks;
        int    dev, busy;   
        struct Minode *mounted_inode;
        char   name[256]; 
        char   mount_name[64];
} MOUNT;


GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 
MINODE minode[100];
MINODE *root;

PROC *running;
PROC *P0;
PROC *P1;
int ino;
int bno;
//int *running;
int imap, bmap;  // IMAP and BMAP block number
int ninodes, nblocks, nfreeInodes, nfreeBlocks;

//char &command[255] = {'ls', 'cd', 'mkdir','rmdir'};
//char command[1024] = {'ls','cd'};
//#define BLOCK_SIZE        1024
#define BLKSIZE           1024

#define BITS_PER_BLOCK    (8*BLOCK_SIZE)
#define INODES_PER_BLOCK  (BLOCK_SIZE/sizeof(INODE))

// Block number of EXT2 FS on FD
#define SUPERBLOCK        1
#define GDBLOCK           2
#define BBITMAP           3
#define IBITMAP           4
#define INODEBLOCK        5
#define ROOT_INODE        2

// Default dir and regulsr file modes
#define DIR_MODE          0040777 
#define FILE_MODE         0100644
#define SUPER_MAGIC       0xEF53
#define SUPER_USER        0

// Proc status
#define FREE              0
#define BUSY              1
#define READY             2
#define KILLED            3

#define RUNNING           2
// Table sizes
#define NMINODE           100
#define NMOUNT            10
#define NPROC             10
#define NFD               10
#define NOFT              50


#define MAX 255
char *disk;;
//char *path = "/";
char *pathName[MAX];
//char *path1;
// Open File Table

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

char pathname[128], parameter[128], *name[128], cwdname[128];
char names[128][256] = {"mkdir", "cd","ls","creat","rmdir","rm","link","symlink","readlink","quit","unlink","stat","touch","pwd","open",
"close","lseek","pfd","read","cat", "write"};

//MINODE *iget();
int fd;
char *cp;
int iblock;
int i_block;
char buf[BLKSIZE],buf2[BLKSIZE];
int ITB, imap, ninodes,nblocks,nfreeInodes,nfreeBlocks;
int get_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  read(fd, buf, BLKSIZE);
}
int put_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);

  write(fd, buf, BLKSIZE );

}


int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit % 8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}

int set_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit % 8;
  buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit % 8;
  buf[i] &= ~(1 << j);
}

int decFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}
int decFreeBlock(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, 2, buf);
}
int incFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}
int incFreeBlock(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count++;
  put_block(dev, 2, buf);
}
int ialloc(int dev)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       decFreeInodes(dev);

       put_block(dev, imap, buf);

       return i+1;
    }
  }
  printf("ialloc(): no more free inodes\n");
  return 0;
}


void idealloc(int dev, int ino){
  get_block(dev,IBITMAP,buf);
  clr_bit(buf,ino-1);
  incFreeInodes(dev);
  put_block(dev,IBITMAP,buf);
}


int balloc(int dev)
{
  int  i;
  int icount;
  //char buf[BLKSIZE];
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  icount = sp->s_blocks_count;
  get_block(dev, 2, buf);
  gp = (GD *)buf;

  printf("icount = %d\n",icount );
  // read inode_bitmap block
  printf("gp->bg_block_bitmap = %d\n", gp->bg_block_bitmap);
  get_block(dev, (gp->bg_block_bitmap), buf);

  for (i=0; i < icount*128; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       decFreeBlock(dev);

       put_block(dev, (gp->bg_block_bitmap), buf);
       printf("b alloc a new block :%d\n",i+1 );
       return i+1;
    }
  }
  printf("balloc(): no more free blocks.\n");
  return 0;
}

void bdealloc(int dev, int bno){
  get_block(dev,BBITMAP,buf);
  clr_bit(buf,ino-1);
  incFreeBlock(dev);
  put_block(dev,BBITMAP,buf);
}






int getino(int dev, char *path){
  //printf("Entering getino\n");
  char buf1[BLKSIZE];
  char buf2[BLKSIZE];
  char *token;
  int o = 0;
  int i;
  int inumber = 10;
  int blk_number;
  int ino= 0;
  char mypath[128];
  strncpy(mypath,path,128);

  //printf("%s\n", mypath);
  token = strtok(mypath, "/");
  //printf("ino: %d\n",ino );
  while(token != NULL){

    pathName[o] = token;
    token = strtok(NULL, "/");
    o++;
  }// Save username and ID into myAcount.
  

  int find =0;
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  iblock = gp->bg_inode_table;   // get inode start block#
  get_block(dev, iblock, buf);
  ip = (INODE *)buf + 1;         // ip points at 2nd INODE  
  i_block = ip->i_block[0];

  get_block(fd, i_block,buf1);
  dp = (DIR *)buf1;
  cp = buf1;
  //printf("o equal to %d\n", o);
  for(i = 0;i<o;i++){

     while((cp<(buf1+BLKSIZE))&(strcmp(pathName[i],dp->name)!=0)){
        //printf("%8d   %8d    %8d     %s\n",dp->inode, dp->rec_len, dp->name_len,dp->name);
        if(dp->rec_len > BLKSIZE){
          //printf("name %s does not exist. \n",pathName[i]);
          exit(0);
        }    
        cp += dp->rec_len;   
        dp = (DIR *)cp;   
      }      
      if (strcmp(pathName[i],dp->name)!=0){
          //printf("can't find %s: \n",pathName[i]);
          ino = 0;
          return 0;
        }
      //printf("Find %s, ino =%d\n",pathName[i] ,dp->inode);  
      
      ino = dp->inode;
      //getchar();    
      inumber = dp->inode - 1;
      
      blk_number = ITB + inumber/8;
      
      get_block(fd, blk_number, buf);
      //printf("inumber = %d, 2 = %d",((inumber % 8)+1), (dp->inode % 8)  );
      ip = (INODE *)buf + (inumber % 8) ;
      i_block = ip->i_block[0];
      
      get_block(fd, i_block,buf1);
      dp = (DIR *)buf1;
      cp = buf1;    
    }
    return ino;
}

findCmd(char *cname){
  int index;
  for (index = 0; index<128;index++){
    if (strcmp(cname,&names[index])==0){
      return index;
    }
  }
  return -1;
}

init(){

  int i;
  P0 = calloc(1,sizeof(PROC));
  P1 = calloc(1,sizeof(PROC));
  P1->uid = 1;
  P0->uid = 0;
  P0->cwd = 0;
  P1->cwd = 0;
  for(i = 0; i<100; i++)
    minode[i].refCount = 0;
  root = 0;
  printf("Initial Ok...\n");

}

MINODE *iget(int dev, int ino){
  int i;
  int blk_number;
  int inumber;
  for(i=0;i<100;i++){
    if (minode[i].ino == ino){
      //printf("find ino %d in minode[] at position %d\n",ino,i );
      minode[i].refCount+=1;
      return &minode[i];
    }
  }
  //printf("Can not find ino %d in minode[]\n",ino );
  for(i=0;i<100;i++){
    if (minode[i].ino == 0){
      //printf("allocated a free minode[%d]\n",i );
      //printf("In iget!!!!! ITB : %d\n",ITB );
      inumber = ino -1;
      blk_number = ITB + inumber/8;
      //printf("In iget!!!!! new block number : %d\n",blk_number );
      get_block(dev, blk_number, buf);
      ip = (INODE *)buf + (inumber % 8);
      minode[i].INODE = *ip;
      minode[i].dev = dev;
      minode[i].ino = ino;
      minode[i].refCount =1;
      minode[i].dirty = 0;
      return &minode[i];
    }
  }

}

int iput(MINODE *mip){
  //printf("iput...\n");
  int i;
  int blk_number;
  int inumber;
  //printf("mip->ino: %d\n",mip->ino);
  //printf("mip->INODE.: %d\n",mip->ino);
  mip->refCount-= 1;
  if((mip->refCount!=0)&&(mip->dirty == 0)){
    //printf("Its clean or somebody else is using..\n");
    return;
  }
      
  else{
    //printf("need to write back...\n");
    inumber = mip->ino - 1;
    blk_number = ITB + inumber/8;
    get_block(mip->dev, blk_number, buf);

    ip = (INODE *)buf + (inumber % 8);
    *ip = mip->INODE;

    put_block(mip->dev,blk_number,buf );

    return;
   }

}


mount_root(char *disk){
  //disk = argv[1];
  fd = open(disk, O_RDWR);
  if (fd < 0){
    printf("open failed\n");
    exit(1);
  }
  get_block(fd, 2, buf); 
  gp = (GD *)buf;
  ITB = gp->bg_inode_table;
  get_block(fd, 2, buf);
  gp = (DIR *)buf;
  imap = gp->bg_inode_bitmap;
  printf("imap = %d\n",imap );
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;
  nfreeInodes = sp->s_free_inodes_count;
  nfreeBlocks = sp->s_free_blocks_count;
  if (sp->s_magic != 0xef53){
    printf("It's not a EXT2 file!!!!!!\n");
    exit(0);
  }
  root = iget(fd,2);
  P0->cwd = root;
  P1->cwd = root;
  running = P0;
  //printf("running->cwd->dev = %d\n",running->cwd->dev );
  printf("Mount Ok....\n");
  
}




ls(char *pathname){
  int inumber, blk_number;
  int i = 0;
  int x = 0;
  int i_block;
  char ftime[64];
  char buf4[BLKSIZE];
  //int ino; 
  int dev = running->cwd->dev;
  //int dev;
  MINODE *mip = running->cwd; 
  //printf("runing dev'ino= %d\n", running->cwd->ino);
  //printf("fd = %d\n",fd );
  // printf("pathname = %s\n", pathname );
  // if (pathname==0){
  //   pathname = "/";
  // }
  //printf("pathname = %s\n", pathname );
  //printf("pathname[0] = %c\n",pathname[0] );
  if (pathname){   // ls pathname:
      if (pathname[0]=='/') {
          dev = root->dev;
        }
      //printf("pathname: %s\n",pathname );
      
      //printf("ino get from ls: %d\n",  ino);
      if (pathname[0]<=0){
        //printf("ls current dir\n");
        dev = running->cwd->dev;
      }
      ino= running->cwd->ino;
      mip = iget(dev, ino);
      //printf("ino = %d\n",ino );
      //printf("mip->refCount = %d\n", mip->refCount);
      //iput(mip);
      
      //printf("i_block[0]=%d\n", mip->INODE.i_block[0]);
      //i_block = mip->INODE.i_block[0];
    
  }



  //printf("i_block= %d\n",i_block );
  while(mip->INODE.i_block[x] != 0){
    //printf("i_block[%d]= %d\n",x,mip->INODE.i_block[x] );
    //getchar();
    get_block(fd, mip->INODE.i_block[x],buf);
    dp = (DIR *)buf;
    cp = buf;  
    //iput(mip); 

    while(cp < (buf+BLKSIZE)){
          //printf("%d   ",dp->name_len);
          inumber = dp->inode - 1;
          blk_number = ITB + inumber/8;
          get_block(fd, blk_number, buf4);
          ip = (INODE *)buf4 + (inumber % 8);
          if ((ip->i_mode & 0xF000) == 0x8000)
            printf("%c",'-');
          if ((ip->i_mode & 0xF000) == 0x4000)
            printf("%c",'d');
          if ((ip->i_mode & 0xF000) == 0xA000)
            printf("%c",'l');

          for (i=8; i >= 0; i--){
            if (ip->i_mode & (1 << i))
          printf("%c", t1[i]);
            else
          printf("%c", t2[i]);
          }
          printf("  ");

          //printf("%4d ",ip->i_nlink);
          // printf("%4d ",ip->i_gid);
          // printf("%4d ",ip->i_uid);
           printf("%8d ",ip->i_size);

          // print time
          strcpy(ftime, ctime(&ip->i_ctime));
          ftime[strlen(ftime)-1] = 0;
          printf("%s  ",ftime);
          printf("%s   ",dp->name);
          printf("\n");

          cp += dp->rec_len;   
          dp = (DIR *)cp;   
          //getchar();

      } 
    x++;
    }

    printf("\n");
    //printf("mip->refCount = %d\n", mip->refCount); 
    iput(mip);
    ino = 0;

}

void cd(char *pathname){
  MINODE *mip;
  if(pathname[0]<=0){
    printf("CD to root...\n");
    running->cwd = iget(root->dev, 2);
    return ;
  }
  if(pathname[0] == "/"){
    running->cwd = iget(root->dev, 2);
    return ;
  }
  //printf("pathname to cd =%s\n",pathname );
  ino = getino(fd,pathname);
  //printf("ino of pathname = %d\n",ino );
  if (ino == 0){
    printf("The pathname: %s does not exist\n",pathname );
    return ;
  }
  mip = iget(root->dev, ino);
  if ((mip->INODE.i_mode & 0xF000) == 0X8000 ){
    iput(mip);
    printf("Can not cd to a file\n");
    return ;
  }
  running->cwd = mip;
  printf("cd to %s\n", pathname );
  return  ;
}


MINODE *mialloc(){
  int i;
  for (i =0; i<100; i++){
    if (minode[i].refCount==0){
      //printf("find free minode at minode[%d]\n",i );
      return &minode[i];
    }
  }
}

int midealloc(MINODE *mip){
  mip->refCount=0;
}


mkDir(char *pathname){
  int dev;
  int pino;
  char *temp;
  MINODE *pip;
  MINODE *mip;
  char *parent;
  char *child;
  temp = calloc(1,255);
  //printf("in mkDir\n");
  //printf("pathname: %s\n",pathname );
  if (pathname[0] == '/'){
    mip = root;
    dev = root->dev;
  }
  else{
    mip = running->cwd;
    dev = running->cwd->dev;
  }
  strcat(temp,pathname);
  ino = getino(dev, temp);
  //printf("child's ino %d\n", ino);
  if(ino!= 0){
    printf("%s already exist\n", temp);
    return;
  } 
  //getchar();
  parent = dirname(pathname);
  //getchar();
  child = basename(temp);
  //printf("parent = %s\n",parent );
  //getchar();
  //printf("child = %s\n",child );
  pino = getino(dev, parent);
  //printf("got parent's ino: %d\n",pino );
  
  pip = iget(dev, pino);
  //printf("got parent's ino: %d\n",pino );
  if(pino == 0){
    printf("invalid dir name\n");
    return;
  } 
  if( (pip->INODE.i_mode & 0xF000) != 0x4000){
    printf("Parent is not a dir\n");
    return;                                       
  }
  //printf("pip->INODE.i_block[i]: %d\n", pip->INODE.i_block[0]);
  mymkdir(pip, child);
  pip->INODE.i_links_count+=1;
  pip->dirty=1;
  //printf("pip->ino : %d\n", pip->ino);
  iput(pip);
  //printf("mip->ino : %d\n", mip->ino);

}

int mymkdir(MINODE *pip, char *name){
  //printf("Entering mymkdir\n");
  MINODE *mip;
  int dev, i;
  dev = pip->dev;
  get_block(dev,2,buf);
  gp = (GD *)buf;

  ino = ialloc(dev);
  //printf("alloc a new inumber %d\n",ino );
  //printf("bg_free_inodes_count: %d\n", gp->bg_free_inodes_count);
  bno = balloc(dev);
  //printf("alloc a new bnumber %d\n",bno );
  mip = iget(dev,ino);
  //printf("Get ino: %d into mip\n", ino);
  INODE *ip = &mip->INODE;
  ip->i_mode = 0x41ED;    // OR 040755: DIR type and permissions
  ip->i_uid  = running->uid;  // Owner uid 
  ip->i_gid  = running->gid;  // Group Id
  ip->i_size = BLKSIZE;   // Size in bytes 
  ip->i_links_count = 2;          // Links count=2 because of . and ..
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
  ip->i_blocks = 2;                 // LINUX: Blocks count in 512-byte chunks 
  ip->i_block[0] = bno;             // new DIR has one data block 
  for (i = 1;i<15;i++){
    ip->i_block[i]=0;
  } 
  //printf("initial ip ok...\n");
  
  mip->dirty = 1;
  //getchar();
  iput(mip);
  //printf("iput mip ok...\n");
  memset(buf,0,BLKSIZE);
  dp = (DIR *)buf;
  cp = buf;
  dp->inode = ino;
  strcpy(dp->name, ".");
  dp->rec_len = 12;
  dp->name_len=1;
  cp+=dp->rec_len;
  dp = (DIR *)cp;
  dp->inode = pip->ino;
  strcpy(dp->name, "..");
  dp->rec_len= BLKSIZE - 12;
  dp->name_len = 2;
  put_block(dev,bno,buf);
  //printf("mkdir ok, only need to write name in it\n");
  enter_name(pip, ino, name);

  //printf("mkdir ok!!!\n");
  //return;


}

int ideal_len(int len_name){
  int base;
  int need_len;
  base = ( 8 + len_name + 3)/4;
  need_len = base * 4;
  return need_len;
}

int enter_name(MINODE *pip,int myino,char *myname){
  //printf("Entering name : %s\n",myname);
  int i;
  int remain, ideal, need;
  for (i = 0; i<12; i++){
    if (pip->INODE.i_block[i]==0)
      break;
  }

  get_block(pip->dev,pip->INODE.i_block[i-1],buf);
  //printf("getting block: %d\n", pip->INODE.i_block[i-1]);
  //dp = (DIR *)buf;
  dp = (DIR *)buf;
  cp = buf;
  while ((cp+ dp->rec_len)< buf + BLKSIZE){
    cp += dp->rec_len;
    
    dp = (DIR *)cp;

  }
  //printf("last entry's name: %s, and rec_len: %d\n",dp->name,dp->rec_len );
  ideal = ideal_len(dp->name_len);
  need = ideal_len(strlen(myname));
  //printf("ideal = %d, need = %d\n",ideal,need );
  remain = dp->rec_len - ideal;
  //printf("remain = %d\n",remain );
  if(remain >= need){
    //printf("no need to alloc a new block...\n");
    dp->rec_len = ideal;
    cp += dp->rec_len;
    dp = (DIR *)cp;
    strncpy(dp->name,myname,strlen(myname));
    dp->rec_len = remain;
    dp->name_len = strlen(myname);
    dp->inode = myino;
    //printf("i will put block in pip->INODE.i_block[%d] : %d ,and dev is %d\n",i-1,pip->INODE.i_block[i-1],pip->dev);
    //put_block(pip->dev, pip->INODE.i_block[i-1],buf);
    put_block(pip->dev, pip->INODE.i_block[i-1],buf);
    //printf("put_block ok...\n");
  }
  else{
    //printf("need new block...\n");
    pip->INODE.i_block[i]= balloc(pip->dev);
    //printf("new block[%d]: %d\n",i, pip->INODE.i_block[i]);
    get_block(pip->dev,pip->INODE.i_block[i],buf);
    dp = (DIR *)buf;
    //printf("myname : %s\n",myname );
    strcpy(dp->name, myname);
    dp->rec_len=BLKSIZE;
    dp->name_len=strlen(myname);
    dp->inode = myino;
    pip->INODE.i_size += BLKSIZE;
    put_block(pip->dev, pip->INODE.i_block[i],buf);
    //printf("i will put block in pip->INODE.i_block[%d] : %d ,and dev is %d\n",i-1,pip->INODE.i_block[i],pip->dev);
    //printf("put block ok...\n");
  }
  //printf("write name ok!!!\n");
  //return;
  
}

creat_file(char *pathname){
  int dev;
  int pino;
  char *temp;
  MINODE *pip;
  MINODE *mip;
  char *parent;
  char *child;
  temp = calloc(1,255);
  //printf("in creat_file\n");
  //printf("pathname: %s\n",pathname );
  if (pathname[0] == '/'){
    mip = root;
    dev = root->dev;
  }
  else{
    mip = running->cwd;
    dev = running->cwd->dev;
  }
  strcat(temp,pathname);
  ino = getino(dev, temp);
 // printf("child's ino %d\n", ino);
  if(ino!= 0){
    printf("%s already exist\n", temp);
    return;
  } 
  parent = dirname(pathname);
  child = basename(temp);

  pino = getino(dev, parent);

  
  pip = iget(dev, pino);
  //printf("got parent's ino: %d\n",pino );
  if(pino == 0){
    printf("invalid dir name\n");
    return;
  } 
  if( (pip->INODE.i_mode & 0xF000) != 0x4000){
    printf("Parent is not a dir\n");
    return;                                       
  }
  //printf("pip->INODE.i_block[i]: %d\n", pip->INODE.i_block[0]);
  my_creat(pip, child);
  pip->INODE.i_links_count+=1;
  pip->dirty=1;
  //printf("pip->ino : %d\n", pip->ino);
  iput(pip);
  //printf("mip->ino : %d\n", mip->ino);

}

int my_creat(MINODE *pip, char *name){
  ///printf("Entering my_creat\n");
  MINODE *mip;
  int dev, i;
  dev = pip->dev;
  get_block(dev,2,buf);
  gp = (GD *)buf;

  ino = ialloc(dev);
  //printf("alloc a new inumber %d\n",ino );
  //printf("bg_free_inodes_count: %d\n", gp->bg_free_inodes_count);
  bno = balloc(dev);
  //printf("alloc a new bnumber %d\n",bno );
  mip = iget(dev,ino);
  //printf("Get ino: %d into mip\n", ino);
  INODE *ip = &mip->INODE;
  ip->i_mode = 0x81A4;    // OR 040755: DIR type and permissions
  ip->i_uid  = running->uid;  // Owner uid 
  ip->i_gid  = running->gid;  // Group Id
  ip->i_size = 0;   // Size in bytes 
  ip->i_links_count = 1;          // Links count=2 because of . and ..
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
  ip->i_blocks = 2;                 // LINUX: Blocks count in 512-byte chunks 
  ip->i_block[0] = 0;             // new DIR has one data block 
  for (i = 1;i<15;i++){
    ip->i_block[i]=0;
  } 
  //printf("initial ip ok...\n");
  
  mip->dirty = 1;
  //getchar();
  iput(mip);
  //printf("iput mip ok...\n");

  enter_name(pip, ino, name);

  //printf("creat_file ok!!!\n");
  //return;
}

rmdir(char *pathname){
  int dev;
  int pino,i;
  char *temp;
  MINODE *pip;
  MINODE *mip;
  char *parent;
  char *child;
  temp = calloc(1,255);
  //printf("in rmdir\n");
  //printf("pathname: %s\n",pathname );
  if (pathname[0] == '/'){
    mip = root;
    dev = root->dev;
  }
  else{
    mip = running->cwd;
    dev = running->cwd->dev;
  }
  strcat(temp,pathname);
  ino = getino(dev, temp);
  parent = dirname(pathname);
  //getchar();
  child = basename(temp);
  //printf("child's ino %d\n", ino);
  pino = getino(dev,parent);
  pip = iget(dev,pino);
  if(ino == 0){
    printf("%s does not exist\n", temp);
    return;
  } 
  mip = iget(dev,ino);
  if( (mip->INODE.i_mode & 0xF000) != 0x4000){
    printf("%s is not a dir\n", pathname);
    iput(mip);
    return -1;                                       
  }
  if (mip->refCount > 1){
    //printf("\n");
    printf("%s is busy!\n",pathname );
    iput(mip);
    return -1;
  }
  if(mip->INODE.i_links_count > 2){
    printf("Dir is not empty, can not remove\n");
    iput(mip);
    return -1;
  }
  else
    for (i = 0; i<12; i++){
      if (pip->INODE.i_block[i]==0)
      break;
    }
    if(i ==0)
      i+=1;
    get_block(pip->dev,mip->INODE.i_block[i-1],buf);
    //printf("getting block: %d\n", pip->INODE.i_block[i-1]);
    dp = (DIR *)buf;
    dp = (DIR *)buf;
    cp = buf;
    while ((cp+ dp->rec_len)< buf + BLKSIZE){
      cp += dp->rec_len;
      // printf("this while?\n");
      // getchar();
      dp = (DIR *)cp;
    }
    if(strcmp(dp->name,"..")!=0){
      printf("Dir is not empty, can not remove\n");
      iput(mip);
      return -1;
    }



  for (i=0; i<12; i++){
         if (mip->INODE.i_block[i]==0)
             continue;
         bdealloc(mip->dev, mip->INODE.i_block[i]);
    }
  idealloc(mip->dev, mip->ino);
  iput(mip); 

  rm_child(pip, child);
  pip->refCount -=1;
  pip->INODE.i_atime = pip->INODE.i_ctime = pip->INODE.i_mtime = time(0L);
  pip->dirty =1;
  pip->INODE.i_links_count-=1;
  pip->dirty=1;
  iput(pip);
  //printf("rm success!!\n");

}

int rm_child(MINODE *pip, char *name){
  //printf("in rm_child\n");
  int length,len, last_len;
  int i = 0;
  int *np;
  int found = 0;
  char temp[255] = "0";
  
  while(found != 1){


    get_block(pip->dev, pip->INODE.i_block[i],buf);
    //printf("pip->INODE.i_block[%d] = %d\n",i, pip->INODE.i_block[i]);
    dp = (DIR *)buf;
    cp = dp;
    while(cp<(buf+BLKSIZE)){
      //printf("name = %s, name_len = %d temp = %s, strcmp(name,temp) = %d \n",name, dp->name_len, temp,strcmp(name,temp));
      //getchar();
      if (strcmp(name,temp)==0){
        //printf("found %s\n",name );
        len = dp->rec_len;
        found=1;
        break;
          //break;//if found the name, break
      }
        //printf("cp= %d, dp->rec_len=%d, dp->name: %s\n",cp, dp->rec_len,dp->name );
      cp+=dp->rec_len;
      last_len = dp->rec_len;
      dp = (DIR *)cp;
      //printf("copy dp->name:%s to temp:%s by %d\n",dp->name,temp,dp->name_len );
      strncpy(temp, dp->name,dp->name_len);
        //printf("cp= %d, dp->rec_len=%d, dp->name: %s\n",cp, dp->rec_len,dp->name );
    }
    i++;
  }

  
  if(dp->rec_len != 1024){// check if this is the first entry of the block
    if((cp+dp->rec_len)==(buf+BLKSIZE)){
      //printf("last entry...\n");
      //printf("last_len = %d\n", last_len);
      //printf("len = %d\n",len );
      cp-=last_len;
      dp = (DIR *)cp;
      dp->rec_len += len;

    }
    else{
      //printf("start working on parent's dir..\n");
      np = cp+dp->rec_len;
      len = dp->rec_len;
      //rintf("buf = %d, BLKSIZE = %d, np = %d\n",buf, BLKSIZE,np );
      length = (int)(buf+BLKSIZE - (int)np);
      //  printf("length = %d\n",length );
      memcpy(cp, np, length);
      dp = (DIR *)cp;
      while((cp+dp->rec_len+len)<(buf+BLKSIZE)){
        //printf("cp= %d, dp->rec_len=%d, dp->name: %s\n",cp, dp->rec_len,dp->name );
        //getchar();
        cp+=dp->rec_len;
        dp = (DIR *)cp;
        }
      dp->rec_len+=len;
    }
  }
  else{
    bdealloc(pip->dev,pip->INODE.i_block[i-1]);
  }
  //printf("put block: %dback \n",pip->INODE.i_block[i-1] );// since i++ at the end of while loop, the real block should be block[i-1]
  put_block(pip->dev,pip->INODE.i_block[i-1],buf);
  pip->dirty=1;
}

rm(char *pathname){
  int dev;
  int pino,i;
  char *temp;
  MINODE *pip;
  MINODE *mip;
  char *parent;
  char *child;
  temp = calloc(1,255);
  printf("in rm\n");
  printf("pathname: %s\n",pathname );
  if (pathname[0] == '/'){
    mip = root;
    dev = root->dev;
  }
  else{
    mip = running->cwd;
    dev = running->cwd->dev;
  }
  strcat(temp,pathname);
  ino = getino(dev, temp);
  parent = dirname(pathname);
  //getchar();
  child = basename(temp);
  printf("child's ino %d\n", ino);
  pino = getino(dev,parent);
  pip = iget(dev,pino);
  if(ino == 0){
    printf("%s does not exist\n", temp);
    return;
  } 
  mip = iget(dev,ino);
  if( (mip->INODE.i_mode & 0xF000) != 0x8000){
    printf("%s is not a file\n", pathname);
    iput(mip);
    return -1;                                       
  }

  idealloc(mip->dev, mip->ino);
  iput(mip); 

  rm_child(pip, child);
  pip->refCount -=1;
  pip->INODE.i_atime = pip->INODE.i_ctime = pip->INODE.i_mtime = time(0L);
  pip->dirty =1;
  //pip->INODE.i_links_count-=1;
  pip->dirty=1;
  iput(pip);
  printf("rm success!!\n");

}


link(char *oldFileName, char *newFileName){
  int dev,npino;
  MINODE *mip;
  MINODE *npip;
  char *parent;
  char *child;
  char temp[255];
  if (pathname[0] == '/'){
    mip = root;
    dev = root->dev;
  }
  else{
    mip = running->cwd;
    dev = running->cwd->dev;
  }
  ino= getino(dev, oldFileName);
  mip = iget(dev, ino);
  if( (mip->INODE.i_mode & 0xF000) == 0x4000){
    printf("Can not link a dir!\n");
    iput(mip);
    return -1;
    }
  if (getino(dev,oldFileName)==0){
    printf("%s does not exist\n",newFileName );
    iput(mip);
    return -1;
  }                              
  if (getino(dev,newFileName)!=0){
    printf("%s already exist\n",newFileName );
    iput(mip);
    return -1;
  }

  strcat(temp,newFileName);
  parent = dirname(newFileName);
  printf("parent = %s\n",parent );
  //getchar();
  child = basename(newFileName);
  printf("child = %s\n",child );
  //getchar();
  if (getino(dev,parent)==0){
    printf("%s does not exist\n",parent );
    iput(mip);
    return -1;
  }
  npino = getino(dev, parent);
  npip = iget(dev,npino);
  enter_name(npip, ino, child);
  mip->INODE.i_links_count +=1;
  iput(mip);

}

symlink(char *oldFileName, char *newFileName){
  int dev,npino;
  MINODE *mip;
  MINODE *npip;
  char *parent;
  char *child;
  char temp[255];
  if (pathname[0] == '/'){
    mip = root;
    dev = root->dev;
  }
  else{
    mip = running->cwd;
    dev = running->cwd->dev;
  }
  if (getino(dev, oldFileName) == 0){
    printf("%s does not exist\n", oldFileName);
    return -1;
  }
  creat_file(newFileName);
  ino = getino(dev, newFileName);
  mip = iget(dev, ino);
  mip->INODE.i_mode = 0xA1A4;
  memcpy(mip->INODE.i_block,oldFileName,strlen(oldFileName));
  iput(mip);

}

touch(char *pathname){
  int dev;
  MINODE *mip;
    if (pathname[0] == '/'){
    mip = root;
    dev = root->dev;
  }
  else{
    mip = running->cwd;
    dev = running->cwd->dev;
  }
  ino = getino(dev,pathname);
  if (ino == 0)
  {
    mkDir(pathname);
  }
  else{
    mip = iget(dev, ino);
    mip->INODE.i_atime = mip->INODE.i_ctime= mip->INODE.i_mtime= time(0L);
    iput(mip);
  }

}

mystat(char *pathname){
  int dev,i;
  MINODE *mip;
  char ftime[64];
  char atime[64];
  char mtime[64];
  ino = getino(dev,pathname);
  mip = iget(dev, ino);
  if ((ip->i_mode & 0xF000) == 0x8000)
      printf("%c",'-');
  if ((ip->i_mode & 0xF000) == 0x4000)
      printf("%c",'d');
  if ((ip->i_mode & 0xF000) == 0xA000)
      printf("%c",'l');

  for (i=8; i >= 0; i--){
    if (ip->i_mode & (1 << i))
      printf("%c", t1[i]);
    else
      printf("%c", t2[i]);
  }
  printf("  ");
  //printf("mode: %d ", mip->INODE.i_mode);
  printf("size: %8d ", mip->INODE.i_size);
  //printf("Blocks %d\n",mip->INODE.i_block );
  printf("uid: %d ", mip->INODE.i_uid);
  printf("gid: %d \n",mip->INODE.i_gid );
  strcpy(atime, ctime(&ip->i_atime));
  atime[strlen(ftime)-1] = 0;
  printf("Access time: %s",atime);
  strcpy(ftime, ctime(&ip->i_ctime));
  ftime[strlen(ftime)-1] = 0;
  printf("Change time: %s\n",ftime);
  strcpy(mtime, ctime(&ip->i_mtime));
  mtime[strlen(ftime)] = 0;
  printf("Motified time: %s\n",mtime);
  
  printf("\n");
  iput(mip);
}

readlink(char *pathname){
  int dev;
  MINODE *mip;
  if (pathname[0] == '/'){
    mip = root;
    dev = root->dev;
  }
  else{
    mip = running->cwd;
    dev = running->cwd->dev;
  }
  ino = getino(dev,pathname);
  mip = iget(dev, ino);
  if(mip->INODE.i_mode != 0xA1A4){
    printf("%s is not a symbolic link\n", pathname);
    return -1;
  }
  printf("%s\n",mip->INODE.i_block );
}

unlink(char *pathname){
  int dev,pino;
  char *parent;
  char *child;
  MINODE *mip, *pip;
  if (pathname[0] == '/'){
    mip = root;
    dev = root->dev;
  }
  else{
    mip = running->cwd;
    dev = running->cwd->dev;
  }
  parent = dirname(pathname);
  //printf("parent = %s\n",parent );
  //getchar();
  child = basename(pathname);
  ino = getino(dev, pathname);
  if(ino == 0){
    printf("%s does not exist\n",pathname );
    return -1;
  }
  mip = iget(dev,ino);
  if((mip->INODE.i_mode & 0xF000) == 0x4000){
    printf("%s is a dir\n",pathname );
    return -1;
  }
  mip->INODE.i_links_count -=1;
  if(mip->INODE.i_links_count == 0){
    truncate(ino);
    idealloc(dev,ino);
  }
  pino = getino(dev,parent);
  pip = iget(dev, pino);
  rm_child(pip, child);
  iput(pip);
  iput(mip);

}

void quit(){
  int i;
  for (i=0;i<100;i++){
    if (minode[i].dirty != 0)
      iput(&minode[i]);
  }
  exit(0);
}

truncate(int ino){
  int dev = running->cwd->dev;
  char buf4[BLKSIZE];
  MINODE *mip;
  mip = iget(dev, ino);
  int index;
  while(mip->INODE.i_block[index] != 0){
    bdealloc(dev, mip->INODE.i_block[index]);
    index++;
  }
  mip->INODE.i_atime = mip->INODE.i_ctime= mip->INODE.i_mtime= time(0L);
  mip->INODE.i_size=0;
  mip->dirty = 1;
}

pwd(MINODE *wd, int cino){
  if(wd->ino == root->ino){
    printf("/");
  }
  char buf[1024], *cp,name[64];
  DIR *dp;
  MINODE *pip;
  get_block(fd, wd->INODE.i_block[0], (char *)&buf);
  dp = (DIR *)buf;
  cp = buf + dp->rec_len;
  dp = (DIR *)cp;
  if(wd->ino != root->ino){
    int ino = dp->inode;
    pip = iget(fd, ino);
    pwd(pip,wd->ino);
  }
  if (cino!=0){
    while (dp->inode != cino)
    {
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
    strncpy(name,dp->name,dp->name_len);
    name[dp->name_len] = '\0';
    printf("%s/",name);
  }
  return;
}

int open_file(char *pathname, char *mode){
  int dev, ino, i;
  int myfd, myMode;
  OFT *oftp;
  oftp = malloc(sizeof(OFT));
  MINODE *mip;
  //printf("mode = %s\n", mode);
  myMode = *mode - '0';
  //printf("myMode = %d\n",myMode );
  if((pathname[0] =='\0') || (mode[0] == NULL)){
    printf("need pathname and mode number for open\n");
    printf("0 for RD, 1 for WR, 2 for RW, 4 for append\n");
    return;
  }
  if (pathname[0] == '/'){
    mip = root;
    dev = root->dev;
  }
  else{
    mip = running->cwd;
    dev = running->cwd->dev;
  }
  ino = getino(dev, pathname);
  mip = iget(dev,ino);
  if (!S_ISREG( mip->INODE.i_mode ))
  {
    printf("%s is not a regular file\n", pathname);
    return -1;
  }
  for (i = 0; i<10;i++){
    if (running->fd[i] != NULL){
      printf("fd %d is not NULL\n",i );
      if(running->fd[i]->minodeptr == mip){
        printf("fd %d is what we are looking at\n",i );
        if(running->fd[i]->mode != 0){
          printf("%s is already in writing mode\n", pathname);
          return;
        }
      }
    }
  }
  //printf("mode = %s\n", mode);
  oftp->mode = myMode;
  //printf("oftp->mode: %d\n",oftp->mode );
  oftp->refCount = 1;
  oftp->minodeptr = mip;

  switch(myMode){
    case 0: oftp->offset = 0;
            break;
    case 1: truncate(ino);
            oftp->offset = 0;
            break;
    case 2: oftp->offset = 0;
            break;
    case 3: oftp->offset = mip->INODE.i_size;
            break;
    default: printf("invalid mode \n");
            return(-1);

  }

  for(myfd = 0; myfd < 10; myfd ++){
    //printf("now fd %d\n",myfd );
    if(running->fd[myfd] == NULL){
      //printf("nothing in fd %d\n",myfd );
      break;
    }
    //printf("here\n");
    
  }
  running->fd[myfd] = oftp;
  if(running->fd[myfd] != NULL){
    //printf("running_>fd[myfd] is not null now\n");
  }
  mip->INODE.i_atime = mip->INODE.i_ctime= mip->INODE.i_mtime= time(0L);
  if (oftp->mode >0){
    mip->dirty = 1;
  }
  //printf("open successed, fd: %d\n",myfd );
  return myfd;
}

int close_file(char *pathname){
  int myfd;
  MINODE *mip;
  OFT *oftp;
  oftp = malloc(sizeof(OFT));
  //myfd = *pathname - '0';
  myfd = atoi(pathname);
  printf("myfd: %d\n", myfd);
  if ((myfd< 0) || (myfd>9)){
    printf("fd out of range\n");
    return;
  }
  if (running->fd[myfd] == NULL){
    printf("fd %d is empty\n",myfd );
    return;
  }
  oftp = running->fd[myfd];
  running->fd[myfd] = 0;
  oftp->refCount--;
  if(oftp->refCount > 0){
    printf("fd %d closed",myfd );
    return;
  }
  mip = oftp->minodeptr;
  iput(mip);
  printf("fd %d closed",myfd );
  return 0;
} 

int close_file_int(int myfd){
  //int myfd;
  MINODE *mip;
  OFT *oftp;
  oftp = malloc(sizeof(OFT));
  //myfd = *pathname - '0';
  //myfd = atoi(pathname);
  //printf("myfd: %d\n", myfd);
  if ((myfd< 0) || (myfd>9)){
    printf("fd out of range\n");
    return;
  }
  if (running->fd[myfd] == NULL){
    printf("fd %d is empty\n",myfd );
    return;
  }
  oftp = running->fd[myfd];
  running->fd[myfd] = 0;
  oftp->refCount--;
  if(oftp->refCount > 0){
    printf("fd %d closed",myfd );
    return;
  }
  mip = oftp->minodeptr;
  iput(mip);
  //printf("fd %d closed",myfd );
  return 0;
} 

int my_lseek(char *pathname, char *parameter){
  int myfd, myPosition;
  int oPosition;
  OFT *oftp;
  myfd = *pathname - '0';
  //myPosition = *parameter - '0';
  myPosition = atoi(parameter);
  oftp = malloc(sizeof(OFT));
  if ((myfd < 0) || (myfd>9)){
    printf("fd out of range\n");
    return;
  }
  oftp = running->fd[myfd];
  oPosition = oftp->offset;
  if (myPosition > oftp->minodeptr->INODE.i_size){
    printf("lseek out of range\n");
    return;
  }
  oftp->offset = myPosition;
  return oPosition;
}

int pfd(){
  int i;
  printf("     fd     mode     offset       INODE\n");
  for (i = 0;i<10;i++){
    if(running->fd[i] != NULL){
      printf("     %d       %d         %d          [%d,%d]\n",i,running->fd[i]->mode,running->fd[i]->offset,
        running->fd[i]->minodeptr->dev,running->fd[i]->minodeptr->ino );
    }
  }
}

int read_file(char *pathname, char *parameter){
  int myfd, nbytes;
  if((pathname[0] =='\0') || (parameter[0] == NULL)){
    printf("need fd and number of bytes to read\n");
    return;
  }
  myfd = atoi(pathname);
  nbytes = atoi(parameter);
  if ((myfd< 0) || (myfd>9)){
    printf("fd out of range\n");
    return;
  }
  if (running->fd[myfd] == NULL){
    printf("fd %d is empty\n",myfd );
    return;
  }
  if ((running->fd[myfd]->mode == 1) || (running->fd[myfd]->mode == 3))
  {
    printf("fd:%d is not in read mode;\n",myfd);
    return;
  }
  return myRead(myfd, buf, nbytes);
}

int myRead(int fd, char buf[], int nbytes){
  //printf("Entering myRead\n");
  int lbk, start,remain,avil, blk;
  int count = 0;
  int *bp;
  //int maxRead;
  char readBuf[BLKSIZE];
  OFT *oftp;
  oftp = malloc(sizeof(OFT));
  MINODE *mip;
  //mip = malloc(sizeof(MINODE));
  oftp = running->fd[fd];
  mip = oftp->minodeptr;
  
  avil = oftp->minodeptr->INODE.i_size - oftp->offset;
  //printf("avil = %d\n", avil);
  //printf("nbytes = %d\n",nbytes );
  // if(nbytes >= avil)
  //   nbytes = avil;
  char *cq = buf;
  while (nbytes && avil){
    //printf("Entering first while loop;\n");
    lbk = oftp->offset / BLKSIZE;
    //printf("==========**************************************************LBK = %d%d%d=======================*******************\n",lbk,lbk,lbk );
    //printf("logical block size = %d\n",lbk );
    start = oftp->offset % BLKSIZE;
    //printf("start = %d\n",start );
    //getchar();
    if(lbk < 12){
      //printf("direct block\n");
      //printf("size = %8d ",oftp->minodeptr->INODE.i_size);
      blk = oftp->minodeptr->INODE.i_block[lbk];//map logical block to physical block;
      //printf("==========================================direct block======================================\n");
    }
    else if (lbk >=12 && lbk < 268){
      // indirect block
      //printf("===========================================indirect block=======================================\n");
      //printf("===========================================oftp->minodeptr->INODE.i_block[12]: %d=======================================\n",oftp->minodeptr->INODE.i_block[12]);
      //printf("the lbk i am getting is!!!!!!!!!!!:%d\n", lbk);
      get_block(oftp->minodeptr->dev, oftp->minodeptr->INODE.i_block[12], buf2);
      bp = buf2;
      blk=bp[lbk-12];
      //printf("the blk i am getting is!!!!!!!!!!!:%d\n", blk);
      
    }
    else
    {
      //printf("==========================================double indirect block================================\n");
      // double indirect
      get_block(oftp->minodeptr->dev, ip->i_block[13], buf2);
      bp =buf2;
      //printf("==========************************************************** fist LBK = %d=======================*******************\n",(((lbk-12) / 256)-1));
      get_block(oftp->minodeptr->dev, bp[((lbk-12) / 256)-1], buf2);
      bp =buf2;
      //printf("==========************************************************** second LBK = %d=======================*******************\n",((lbk-12) % 255));
      //getchar();
      blk = bp[(lbk-12) % 256];
      //printf("==========================================double indirect block================================\n");
    }
    // get the data block into buffer;
    //printf("the blk i am getting is!!!!!!!!!!!:%d\n", blk);
    get_block(oftp->minodeptr->dev, blk, readBuf);
    char *cp = readBuf + start;
    remain = BLKSIZE - start;
    //maxRead = nbytes % BLKSIZE;
    //printf("remain = %d\n",remain );
    //printf("Start reading\n");
    while(remain > 0){
      *cq++ = *cp++;
      oftp->offset++;
      count++;
      remain--;
      avil--;
      nbytes--;
      //printf("nbytes = %d\n",nbytes );
      if((nbytes <= 0)||(avil <= 0))
        break;
    }
    

  }
  //printf("read %d bytes from file descriptor %d\n",count, fd );
  //printf("count = %d\n",count );
  //getchar();
  return count;
}

cat_file(char *fileName){
  char myBuf[1024], dummy = 0;
  int n;
  int fd = open_file(fileName, "0");
  //printf("fd=%d\n",fd );
  if(fd == -1){
    printf("can not cat this file\n");
    return;
  }
  printf("===================================================================================\n");
  printf("\n\n");
  while( n = myRead(fd, myBuf, 1024)){
    //printf("while in cat\n");
    //printf("n = %d\n",n );
    //getchar();
    myBuf[n] = 0;
    printf("%s", myBuf);
    memset(myBuf,0,sizeof(myBuf));
    //printf("%s", myBuf);
  }
  //printf("here?\n");
  //itoa(fd,myfd[0],0);
  
  close_file_int(fd);
  printf("\n\n");
  printf("====================================================================================\n");
}

int write_file(char *pathname, char *parameter){
  int myfd, nbytes;
  //char writeBuf[BLKSIZE];
  myfd = atoi(pathname);
  nbytes = strlen(parameter);
  strcpy(buf, parameter);
  if ((myfd< 0) || (myfd>9)){
    printf("fd out of range\n");
    return;
  }
  if (running->fd[myfd] == NULL){
    printf("fd %d is empty\n",myfd );
    return;
  }
  if (running->fd[myfd]->mode == 0)
  {
    printf("fd:%d is not in write mode;\n",myfd);
    return;
  }
  myWrite(myfd,buf, nbytes );

}

int myWrite(int fd, char buf[], int nbytes){
  int lbk, start,remain,avil, blk;
  char rbuf[BLKSIZE];
  OFT *oftp;
  oftp = malloc(sizeof(OFT));
  MINODE *mip;
  oftp = running->fd[fd];
  mip = oftp->minodeptr;
  printf("Going to write %s\n",buf );
  char *cq = buf;
  //avil = oftp->minodeptr->INODE.i_size - oftp->offset;
  while(nbytes>0){
    //compute logical blocks and start byte;
    lbk = oftp->offset / BLKSIZE;
    start = oftp->offset % BLKSIZE;
    if (lbk < 12){//direct block
      if(mip->INODE.i_block[lbk] == 0){
        mip->INODE.i_block[lbk] = balloc(mip->dev);
      }
      blk = mip->INODE.i_block[lbk];
    }
    else if(blk >12 && blk <(256+12)){
      // Indirect block
    }
    else {
      // Double indirect block
    }
    get_block(mip->dev, blk, rbuf);
    printf("nbytes:%d\n",nbytes );

    char *cp = rbuf + start;
    remain = BLKSIZE - start;
    while(remain> 0){
      printf("remain = %d\n",remain );
      *cp++ = *cq++;
      nbytes--;remain--;
      oftp->offset++;
      printf("offset = %d\n", oftp->offset);
      printf("i_size = %d\n", mip->INODE.i_size);
      if(oftp->offset > mip->INODE.i_size){
        mip->INODE.i_size++;
      }
      if(nbytes <= 0){
        break;
      }
      put_block(mip->dev, blk, rbuf);
      printf("put....\n");
    }

  }
  mip->dirty = 1;
  iput(mip);
  printf("Wrote %d char into file descriptor %d\n",nbytes, fd );
  return nbytes;
}

main(int argc, char *argv[ ])
{ 

  int ino;
  int o = 0;
  int i,cmd; 
  char line[128], cname[64];
  if (argc < 1){
   disk = "diskimage";
  }
  disk = argv[1];

  init();
  mount_root(disk);
  
  printf("haha : %d\n", P0->cwd->dirty);

  while (1){
    printf("P%d running: ", running->pid);

    /* zero out pathname, parameter */
    for (i=0; i<64; i++){
        pathname[i]=parameter[i] = 0;
    }      
    /* these do the same; set the strings to 0 */
    memset(pathname, 0, 64);
    memset(parameter,0, 64);

    //printf("input command %d: \n",findCmd(cname) );
    gets(line);
    if (line[0]==0) continue;


    sscanf(line, "%s %s %64c", cname, pathname, parameter);
    cmd = findCmd(cname);
    switch(cmd){
           case 0 : mkDir(pathname);                        break;
           case 1 : cd(pathname);                           break;
           case 2 : ls(pathname);                           break;
           case 3 : creat_file(pathname);                   break;
           case 4 : rmdir(pathname);                        break;
           case 5 : rm(pathname);                           break;
           case 6 : link(pathname, parameter);              break;
           case 7:  symlink(pathname, parameter);           break;
           case 8:  readlink(pathname);                     break;
           case 9:  quit();                                 break;
           case 10: unlink(pathname);                       break;
           case 11: mystat(pathname);                       break;
           case 12: touch(pathname);                        break;
           case 13: pwd(running->cwd, 0);printf("\n");      break;
           case 14: open_file(pathname,parameter);          break;
           case 15: close_file(pathname);                   break;
           case 16: my_lseek(pathname,parameter);           break;
           case 17: pfd();                                  break;    
           case 18: read_file(pathname, parameter);         break;
           case 19: cat_file(pathname);                     break;
           case 20: write_file(pathname, parameter);        break;
           default: printf("invalid command\n");
                    break;
      }




  }





}
