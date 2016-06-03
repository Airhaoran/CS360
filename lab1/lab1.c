#include <stdio.h>   
#include <stdlib.h>  
#include <string.h>


typedef unsigned int u32;

#define BASE 10


//int **ip;
//char *cp;
char *table = "0123456789ABCDEF";
//int *ip;
int rpu(u32 x)
{
  char c;
  
  if (x){
     c = table[x % BASE];
     rpu(x / BASE);
     putchar(c);
  }
} 
int rpx(u32 x)
{
  char c;
  if (x){
     c = table[x % 16];
     rpu(x / 16);
     putchar(c);
  }
} 
int rpo(u32 x)
{
  char c;
  if (x){
     c = table[x % 8];
     rpu(x / 8);
     putchar(c);
  }
} 

int printu(u32 x)
{
  if (x==0)
     putchar('0');
  else
     rpu(x);
  putchar(' ');
}

int printd(u32 x)
{
  //printf("We r now in printd, x's address = %d\n", &x);
  if (x==0)
     putchar('0');
  if (x <0){
     putchar('-');
     rpu(x);
     }
  else
     rpu(x);
  putchar(' ');
}

int printo(u32 x)
{
  if (x==0)
     putchar('0');
  else
     rpo(x);
  putchar(' ');
}

int printx(u32 x)
{
  if (x==0)
     putchar('0');
  else
     rpx(x);
  putchar(' ');
}
int prints(char *x)
{
  int i;
  for (i=0;x[i]!='\0';i++){
	putchar(x[i]);
	}
  putchar(' ');
}

int myprintf(char *fmt, ...) // SOME C compiler requires the 3 DOTsint a, int b, int c, int d
{
   
   	
   int *ip;
   char *cp;
  
   printf("&cp = %8x\n", &cp);
   cp = fmt;
   printf("adress of fmt=%8x\n", &fmt);
  
   
   ip =(&fmt)+1;
    
   /*printf("&cp = %8x\n", &cp);
   printf("*cp = %c\n", *cp);    
   printf("ip = %8x\n", ip); */
   printf("*ip = %d\n", *ip);
   

   for (cp ;*cp != '\0'; cp++){
	if (*cp != '%'){
		putchar(*cp);
		continue;
		}
	if(*cp == '\n'){
		putchar(*cp);
		putchar('\r');
		continue;
		}
	else{
		
		switch(*++cp){
			case 'c' : putchar(*ip);  break;
			case 's' : putchar(*ip);  break;
			case 'u' : printu(*ip);  break;
			case 'd' : printd(*ip);  break;
			case 'o' : printo(*ip);  break;
			case 'x' : printx(*ip);  break;
			}
	     }
	ip--;
   
    }
    printf("\n");
}




main()
{
  //myprintf("this %c is a %d test %x\n", 'a', 1234, 100); 
  //asm("movl %ebp, ip");
  //char new;
  //printf("enter a sentence:\n");
  //scanf("%c", &new);
  myprintf("---------- testing YOUR myprintf() ---------\n");
  myprintf("this is a test\n");
  //myprintf("testing a=%x\n", 620);
  myprintf("testing a=%d b=%x c=%c s=%s\n", 123, 123, 'a', "testing");
  myprintf("string=%s, a=%d  b=%u  c=%o  d=%x\n","testing string", -1024, 1024, 1024, 1024);
  //myprintf("mymain() return to main() in assembly\n"); 
}


