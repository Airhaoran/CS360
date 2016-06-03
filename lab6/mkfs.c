/* ================== mkfs.c file =====================================*/
/***************************************************************
    KCW: make ext2 filesystem on 1.44MB floppy disk (11-4-97)
***************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp;

#define BLOCK 1024

char buf[BLOCK], *cp;
int  fd, i, j, n;
int  nblocks, ninodes, ngroups, ngblocks;
int  bmap, imap, inodes_start, inodes_blocks, root_block;
int  free_blocks, free_inodes;
int  used_blocks, used_inodes;


int get_block(int block, char *buf)
{
   lseek(fd,(long)BLOCK*block,0);
    read(fd,buf,BLOCK);
} 

int put_block(int block, char *buf)
{
   lseek(fd, (long)BLOCK*block,0);
   write(fd, buf,BLOCK);
}


int setbit(char *buf, int bit) // set bit_th bit in char buf[1024] to 1
{
  int i,j;
  i = bit / 8; 
  j = bit % 8;
  buf[i] |= (1 << j);
  return 1;
}  

int clearbit(char *buf, int bit) // clear bit_th bit in char buf[1024] to 0
{
  int i, j;
  i = bit / 8; 
  j = bit % 8;
  buf[i] &= ~(1 << j);
  return 1;
}  


int make_super()
{
  printf("     making super block ......\n");
  memset(buf, 0, 1024);
  sp = (SUPER *)buf;

  sp->s_inodes_count = ninodes;                 /* Number of inodes     */
  sp->s_blocks_count = nblocks;                 /* Number of blocks     */
  sp->s_r_blocks_count = 0;                     /* No reserved blocks   */
  sp->s_free_blocks_count = free_blocks;  /* Free blocks count    */
  sp->s_free_inodes_count = free_inodes;  /* Free inodes count    */
  sp->s_first_data_block = 1;             /* First Data Block     */
  sp->s_log_block_size = 0;                     /* Block size           */
  sp->s_log_frag_size  = 0;               /* Fragment size        */
  sp->s_blocks_per_group = 8192;          /* Blocks per group     */
  sp->s_frags_per_group  = 8192;                /* Fragments per group  */
  sp->s_inodes_per_group = ninodes;         /* Inodes per group     */
  sp->s_max_mnt_count = 20;               /* Maximal mount count  */
  sp->s_magic = 0xEF53;                         /* ext2 magic signature */  
  sp->s_inode_size = 128;
  put_block(1, buf);
}

int make_group()
{
  printf("     making group descriptors ......\n");
  memset(buf, 0, 1024);
  gp = (GD *)buf;

  gp->bg_block_bitmap = 3;    /* Blocks bitmap block */
  gp->bg_inode_bitmap = 4;    /* Inodes bitmap block */
  gp->bg_inode_table  = 5;    /* Inodes table block */
  gp->bg_free_blocks_count = free_blocks; /* Free blocks count */
  gp->bg_free_inodes_count = free_inodes; /* Free inodes count */
  gp->bg_used_dirs_count = 1;     /* Directories count */
  put_block(2, buf);
}

int make_bmap()
{
  // nblocks, used_blocks ==> bmap block
  char buf[1024];
  int i;
  memset(buf, 0xFF, 1024);   // set all bits to 1

  for (i=used_blocks-1; i < nblocks-1; i++)
    clearbit(buf, i);
  put_block(bmap, buf);
}
  
int make_imap()
{
  // inodes, used_inodes ==> imap block
  char buf[1024];
  int i;
  //  printf("imap=%d bmap=%d\n", imap, bmap);

  memset(buf, 0xFF, 1024);   // set all bits to 1

  for (i=used_inodes; i < ninodes; i++)
    clearbit(buf, i);
  put_block(imap, buf);
}

int clear_inodes()
{
  int i; char buf[1024];
  memset(buf, 0, 1024);
  for (i=inodes_start; i < inodes_start+inodes_blocks; i++){
       put_block(i, buf);
  }
}

int make_root_dir()
{
  printf("     making root directory ......\n");
  memset(buf, 0, 1024);
  ip = (INODE *)buf; 
  ip++;

  ip->i_mode = 0x41ED;    /* File mode */
  ip->i_uid  = 0;   /* Owner Uid */
  ip->i_size = 1024 ;   /* Size in bytes */
  ip->i_gid =  0;   /* Group Id */
  ip->i_links_count = 2;  /* Links count */
  ip->i_blocks = 2;       /* Blocks count */
  ip->i_block[0] = root_block;  /* Pointers to blocks */
  put_block(inodes_start, buf);
}

int make_root_block()
{ 
  printf("     making root data block ......\n");
  memset(buf, 0, 1024);

  dp = (DIR *)buf;   
  dp->inode = 2;    /* Inode number */
  strcpy(dp->name, ".");        /* File name */
  dp->name_len = 1;   /* Name length */
  dp->rec_len = 12;   /* Directory entry length */

  /*********************************************************************** 
    ext2 dir entries are variable length (min=12 bytes). Last entry's 
    rec_len must be set to the remaining length of the block. 
  ************************************************************************/
  cp = buf; 
  cp += dp->rec_len;            /* advance by rec_len */
  dp = (struct ext2_dir_entry_2 *)cp;
  dp->inode = 2;    /* Inode number */
  dp->name_len = 2;   /* Name length */
  strcpy(dp->name, "..");       /* File name */
  dp->rec_len = 1012;   /* Directory entry length */
  put_block(root_block, buf);
}


main(int argc, char *argv[])
{
  int r;
  if (argc < 3){
      printf("Usage mkfs IMAGENAME nblock [ninodes]\n");
      exit(1);
  } 
  fd = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC, 0644);
  if (fd < 0){
      printf("open %s error\n", argv[1]); 
      exit(1);
  }

  nblocks = atoi(argv[2]);            /* number of blocks */
  if (nblocks > 1440){
      printf("too many blocks for %s\n", argv[1]);
      exit(1);
  }

  lseek(fd, (long)(BLOCK*(nblocks-1)), 0);  /* seek to last block */
  r = write(fd, buf, BLOCK);                /* write a block of 0 */
  if (r < 0){
    printf("write to block %d failed\n", nblocks);  exit(3);
  }

  ngroups = (nblocks + 8191)/8192;    /* number of groups       (1) */
  ngblocks= (ngroups*32 + 1023)/1024; /* number of group blocks (1) */

  ninodes = nblocks/4;                /* default # of inodes */
  if (argc>3)
    ninodes = atoi(argv[3]);

  ninodes = 8*(ninodes/8);
  inodes_blocks = (ninodes+7)/8;      /* number of inodes blocks */

  bmap = 2 + ngblocks;                /* blocks bitmap after group (3) */ 
  imap = bmap + 1;                    /* inodes bitmap after bmap  (4) */
  inodes_start = imap + 1;            /* inodes blocks after imap  (5) */

  root_block = inodes_start + inodes_blocks;   /* data block of root */

  used_blocks = root_block + 1;       /* number of used blocks */
  used_inodes = 10;                   /* inodes 1 to 10 are reserved */

  free_blocks = nblocks - used_blocks;        /* number of free blocks */
  free_inodes = ninodes - 10;                 /* number of free inodes */

  printf("\n************  mkfs on %s  *************\n", argv[1]);
  printf("nblocks      = %4d   ninodes     = %4d\n", nblocks, ninodes);
  printf("inodes_start = %4d   root_block  = %4d\n", inodes_start, root_block);
  printf("free_blocks  = %4d   free_inodes = %4d\n", free_blocks, free_inodes);
  printf("********************************************\n");

  getchar();

  /* make super block */
  make_super();
  getchar();

  /* make group table */
  make_group();
  getchar();

  printf("     making blocks bit map ......\n");
  //make_bitmap(nblocks, used_blocks, bmap);
   make_bmap();

  printf("     making inode bit map ......\n");
  //make_bitmap(ninodes, used_inodes, imap);
  make_imap();

  getchar();

  clear_inodes();
  make_root_dir();
  make_root_block();

  close(fd);

  fd = open(argv[1], O_RDONLY);
  print_fs();
  printf("**************  All Done  ******************\n");
}


int print_bits(c) char c;
{
  char s;
  int i;

  s = 1;
  for (i = 0; i < 8; i++){
      if (c & s) putchar('1');
      else       putchar('0');
      s = s << 1;
  }
  putchar(' ');
}


int print_fs()
{
  int i,j,k;
  int bcount, icount;
  bcount = nblocks;
  icount = ninodes;

  printf("\n******** Blocks Bitmap ********\n");
  get_block(3, buf);

  for (j=0; j < BLOCK; j++){
      if (j && j % 8 == 0) 
          printf("\n");
          print_bits(buf[j]);
          bcount -= 8;
          if (bcount < 0) 
             break;
  }

  printf("\n\n********** Inodes Bitmap ***********\n");
  get_block(4, buf);

  for (j = 0; j < BLOCK; j++){
      if (j && j % 8 == 0) 
         printf("\n");
      print_bits(buf[j]);
      icount -= 8;
      if (icount < 0) 
         break;
  }
  printf("\n");

}