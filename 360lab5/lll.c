ls
clude <stdio.h>   
#include <stdlib.h>  
#include <string.h>

#define N 10

typedef struct node{
  struct node *next;
  char name[64];
  int  id;
}NODE;

char name[64];

NODE *freelist, *mylist, node[N];

int printlist(char *name, NODE *p)
{
   printf("%s = ", name);
   while(p){
     printf("%s -> ", p->name);
      p = p->next;
  }
  printf("NULL\n");
}

int init()
{
  int i;
  for (i=0; i < N; i++){
      node[i].next = &node[i+1];
      node[i].name[0] = 0;
      node[i].id = i;
  }
  freelist = &node[0];
  node[N-1].next = 0;

  mylist = 0;
}

NODE *getnode()
{
  NODE *p;
  p = freelist;
  if (p)
     freelist = freelist->next;
  return p;
}

int putnode(NODE *p)
{
  p->next = freelist;
  freelist = p;
}


char buf[64];
NODE *search();

main()
{
  int i; NODE *p;
  init();

  printlist("freelist", freelist);
  printlist("mylist", mylist);

  printf("enter a key:"); getchar();

  for (i=0; i < N; i++){
     sprintf(buf, "%s%d", "node", i);
     insert(&mylist, buf);
  }
  printlist("mylist", mylist);

  printf("enter a name to search : ");
  gets(buf);
  /**************
  fgets(buf, 64, stdin);
  buf[strlen(buf)-1] = 0;   // kill CR key
  *****************/
  if ( p = search(mylist, buf) )
    printf("found %s at %08x\n", buf, p);
  else
     printf("%s not found\n", buf);

  printlist("mylist", mylist);
  printf("enter a name to delete : ");
  gets(buf);
  /*************
  fgets(buf, 64, stdin);
  buf[strlen(buf)-1] = 0;   // kill CR key
  ************/
  if ( delete(&mylist, buf) )
    printf("deleted OK\n");
  else
     printf("delete failed\n");

  printlist("mylist", mylist);
  printlist("freelist", freelist);
}     


int insert(NODE **list, char *name)
{
  // get a new node
  NODE *p, *q;

  p = getnode();

  if (p==0){
    printf("no more nodes\n");
    return -1;
  }

  strcpy(p->name, name);
  
  if (*list==0){
     *list = p;
     p->next = 0;
     return;
  }

  // insert to end of list
  q = *list;
  while(q->next)
    q = q->next;

  q->next = p;
  p->next = 0;
}

NODE *search(NODE *list, char *name)
{
  NODE *p;
  p = list;
  while(p){
    if (strcmp(p->name, name)==0)
      return p;
    p = p->next;
  }
  return 0;
} 

int delete(NODE **list, char *name)
{
  NODE *p, *q;
  if (*list == 0) 
      return 0;

  p = *list;
  if (strcmp(p->name, name)==0){
    *list = p->next;
    putnode(p);
    return 1;
  }

  q = p->next;
  while(q){
    if (strcmp(q->name, name)==0){
      p->next = q->next;
      putnode(q);
      return 1;
    }
    p = q; 
    q = q->next;
  }
  return 0;
}
     
