#include <stdio.h>   
#include <stdlib.h>  
#include <string.h>

typedef struct node {
    struct node *siblingPtr, *childPtr, *parentPtr;
    char type;
    char name[64];
} NODE;


NODE *searchSibling(NODE *n, char *c){
   //int i = -1;
   NODE *new;
   new = n->siblingPtr;
   while(new->siblingPtr != 0){
      if(new->name == c)
	//new = n->siblingPtr;
	return new;
      else{
        new = n->siblingPtr;
	searchSibling(new, c);
	}
   }
   return 0;
}

int main(){
   char A;
   strcpy(A, "A");	
   NODE *p;
   NODE *c;
   NODE *q;
   NODE *r;
   strncpy(p->name, *A, 64);
   //strcpy(p->name, 'B');
   //strcpy(p->name, 'C'); 
   //strcpy(p->type, 'A');
   //strcpy(p->type, 'B');
   //strcpy(p->type, 'C');
  // p->siblingPtr = c;   
  // c->siblingPtr = q;
   //r = searchSibling(p, "C");
   //printf("the type of finded pointer :%c", c->type);
   
}

