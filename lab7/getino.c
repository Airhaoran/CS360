/********* lab.c*********/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define BLKSIZE 1024
#define EXT2_NAME_LEN 255


// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 

int fd;
char *cp;
int iblock;
int i_block;
char buf[BLKSIZE];


int get_block(int fd, int blk, char buf[ ])
{
  lseek(fd,(long)blk*BLKSIZE, 0);
   read(fd, buf, BLKSIZE);
}


int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit / 8;  j = bit % 8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}

groupD()
{
  // read SUPER block
  get_block(fd, 1, buf);  
  
  //get_block(fd, 1, buf1);  
  sp = (SUPER *)buf;

  // check for EXT2 magic number:

  printf("s_magic = %x\n", sp->s_magic);
  if (sp->s_magic != 0xEF53){
    printf("NOT an EXT2 FS\n");
    exit(1);
  }
  get_block(fd, 2, buf); 
  gp = (GD *)buf;
  printf("EXT2 FS OK\n");

  printf("bg_block_bitmap = %d\n", gp->bg_block_bitmap);
  printf("bg_inode_bitmap = %d\n", gp->bg_inode_bitmap);

  printf("bg_inode_table = %d\n", gp->bg_inode_table);
  printf("bg_free_blocks_count = %d\n", gp->bg_free_blocks_count);
  printf("bg_free_inodes_count = %d\n", gp->bg_free_inodes_count);


  printf("bg_used_dirs_count = %d\n", gp->bg_used_dirs_count);
  printf("bg_reserved[3] = %d\n", gp->bg_reserved[3]);





}

bmap()
{
  char buf[BLKSIZE];
  int  bmap, nblocks;
  int  i;

  // read SUPER block
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  nblocks = sp->s_blocks_count;
  printf("nblocks = %d\n", nblocks);

  // read Group Descriptor 0
  get_block(fd, 2, buf);
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  printf("bmap = %d\n", bmap);

  // read inode_bitmap block
  get_block(fd, bmap, buf);

  for (i=0; i < nblocks; i++){
    if (i && (i % 8)==0)
       printf(" ");
    (tst_bit(buf, i)) ? putchar('1') : putchar('0');
    // if (i && (i % 8)==0)
    //    printf(" ");
  }
  printf("\n");
}

inode()
{
  char buf[BLKSIZE];

  // read GD
  get_block(fd, 2, buf);
  gp = (GD *)buf;


  iblock = gp->bg_inode_table;   // get inode start block#
  printf("inode_block=%d\n", iblock);

  // get inode start block     
  get_block(fd, iblock, buf);

  ip = (INODE *)buf + 1;         // ip points at 2nd INODE
  
  printf("mode=%4x ", ip->i_mode);
  printf("uid=%d  gid=%d\n", ip->i_uid, ip->i_gid);
  printf("size=%d\n", ip->i_size);
  printf("time=%s", ctime(&ip->i_ctime));
  printf("link=%d\n", ip->i_links_count);
  printf("i_block[0]=%d\n", ip->i_block[0]);

}

dir()
{
  char buf[BLKSIZE];

  // read GD
  get_block(fd, 2, buf);
  gp = (GD *)buf;


  iblock = gp->bg_inode_table;   // get inode start block#
  printf("inode_block=%d\n", iblock);

  // get inode start block     
  get_block(fd, iblock, buf);

  ip = (INODE *)buf + 1;         // ip points at 2nd INODE

  i_block = ip->i_block[0];
  printf("i_block = %8d\n", i_block);
  get_block(fd, i_block,buf);
  dp = (DIR *)buf;
  cp = buf;
  printf("   inode    rec_len    name_len   name\n");
  while(dp->inode < 10000){
    //printf("   inode    rec_len    name_len   name\n");
    printf("%8d   %8d    %8d     %s\n",dp->inode, dp->rec_len, dp->name_len,dp->name);
    cp += dp->rec_len;       // advance cp by rec_len BYTEs
    dp = (DIR *)cp;        // pull dp along to the next record
    // printf("Enter to continue: ");
    // getchar();
    //printf("inode = %d\n",dp->inode);
  }
}
imap()
{
  char buf[BLKSIZE];
  int  imap, ninodes;
  int  i;

  // read SUPER block
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  ninodes = sp->s_inodes_count;
  printf("ninodes = %d\n", ninodes);

  // read Group Descriptor 0
  get_block(fd, 2, buf);
  gp = (GD *)buf;

  imap = gp->bg_inode_bitmap;
  printf("bmap = %d\n", imap);

  // read inode_bitmap block
  get_block(fd, imap, buf);

  for (i=0; i < ninodes; i++){
    if (i && (i % 8)==0)
       printf(" ");
    (tst_bit(buf, i)) ? putchar('1') : putchar('0');
    // if (i && (i % 8)==0)
    //    printf(" ");
  }
  printf("\n");
}

super()
{
  // read SUPER block
  get_block(fd, 1, buf);  
  sp = (SUPER *)buf;

  // check for EXT2 magic number:

  printf("s_magic = %x\n", sp->s_magic);
  if (sp->s_magic != 0xEF53){
    printf("NOT an EXT2 FS\n");
    exit(1);
  }

  printf("EXT2 FS OK\n");

  printf("s_inodes_count = %d\n", sp->s_inodes_count);
  printf("s_blocks_count = %d\n", sp->s_blocks_count);

  printf("s_free_inodes_count = %d\n", sp->s_free_inodes_count);
  printf("s_free_blocks_count = %d\n", sp->s_free_blocks_count);
  printf("s_first_data_blcok = %d\n", sp->s_first_data_block);


  printf("s_log_block_size = %d\n", sp->s_log_block_size);
  printf("s_log_frag_size = %d\n", sp->s_log_frag_size);

  printf("s_blocks_per_group = %d\n", sp->s_blocks_per_group);
  printf("s_frags_per_group = %d\n", sp->s_frags_per_group);
  printf("s_inodes_per_group = %d\n", sp->s_inodes_per_group);


  printf("s_mnt_count = %d\n", sp->s_mnt_count);
  printf("s_max_mnt_count = %d\n", sp->s_max_mnt_count);

  printf("s_magic = %x\n", sp->s_magic);

  printf("s_mtime = %s", ctime(&sp->s_mtime));
  printf("s_wtime = %s", ctime(&sp->s_wtime));


}

#define MAX 255
char *disk;;
char *path;
char *pathname[MAX];

main(int argc, char *argv[ ])
{ 
  int ino;
  if (argc < 2){
   printf("Need disk name and path to run\n");
   exit(0);
  }
  disk = argv[1];
  path = argv[2];
  
  //printf("-----------------------------------------------------\n");
  fd = open(disk, O_RDONLY);
  if (fd < 0){
    printf("open failed\n");
    exit(1);
  }

  ino = getino(fd, path);
  printf("ino = %d\n",ino );

}

int min(int a,int b){
  if(a>b)
    return b;
  else
    return a;
}

int getino(int dev, char *path){

  char buf1[BLKSIZE];
  char buf2[BLKSIZE];
  char *token;
  int o = 0;
  int i;
  int inumber = 10;
  int blk_number;
  int ino;
  token = strtok(path, "/");

  while(token != NULL){
    pathname[o]=token;
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
  for(i = 0;i<o;i++){
     printf("search %s: \n",pathname[i]);
     
     while((cp<(buf1+BLKSIZE))&(strcmp(pathname[i],dp->name)!=0)){
        
        if(dp->rec_len > BLKSIZE){
          printf("name %s does not exist. \n",pathname[i]);
          exit(0);
        }    
        cp += dp->rec_len;   
        dp = 
        (DIR *)cp;   
      }      
      if (strcmp(pathname[i],dp->name)!=0){
          printf("can't find %s: \n",pathname[i]);
          exit(0);
        }
     
      printf("Find %s, ino =%d\n",pathname[i] ,dp->inode);  
      
      ino = dp->inode;
      //getchar();    
      inumber = dp->inode - 1;
      
      blk_number = 10 + inumber/8;
      
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