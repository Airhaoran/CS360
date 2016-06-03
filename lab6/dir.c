/********* dir.c: print information in / INODE (INODE #2) *********/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>

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

int get_block(int fd, int blk, char buf[ ])
{
  lseek(fd,(long)blk*BLKSIZE, 0);
   read(fd, buf, BLKSIZE);
}

dir()
{
  char buf[BLKSIZE];

  // read GD
  get_block(fd, 2, buf);
  gp = (GD *)buf;

  /****************
  printf("%8d %8d %8d %8d %8d %8d\n",
	 gp->bg_block_bitmap,
	 gp->bg_inode_bitmap,
	 gp->bg_inode_table,
	 gp->bg_free_blocks_count,
	 gp->bg_free_inodes_count,
	 gp->bg_used_dirs_count);
  ****************/ 
  iblock = gp->bg_inode_table;   // get inode start block#
  printf("inode_block=%d\n", iblock);

  // get inode start block     
  get_block(fd, iblock, buf);

  ip = (INODE *)buf + 1;         // ip points at 2nd INODE
  
  // printf("mode=%4x ", ip->i_mode);
  // printf("uid=%d  gid=%d\n", ip->i_uid, ip->i_gid);
  // printf("size=%d\n", ip->i_size);
  // printf("time=%s", ctime(&ip->i_ctime));
  // printf("link=%d\n", ip->i_links_count);
  // printf("i_block[0]=%d\n", ip->i_block[0]);
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

char *disk = "mydisk";
main(int argc, char *argv[])
{ 
  if (argc > 1)
    disk = argv[1];

  fd = open(disk, O_RDONLY);
  if (fd < 0){
    printf("open %s failed\n", disk);
    exit(1);
  }

  dir();
}