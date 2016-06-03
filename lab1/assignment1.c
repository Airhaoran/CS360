/************* t.c file ********************/
#include <stdio.h>
#include <stdlib.h>

int *FP;

main(int argc, char *argv[ ], char *env[ ])
{
  int a,b,c;
  printf("enter main\n");
  
  printf("&argc=%x argv=%x env=%x\n", &argc, argv, env);
  printf("&a=%8x &b=%8x &c=%8x\n", &a, &b, &c);

  a=1; b=2; c=3;
  A(a,b);
  printf("exit main\n");
}

int A(int x, int y)
{
  int d,e,f;
  printf("enter A\n");
  printf("address of d=%8x, e=%8x, f=%8x\n", &d, &e,&f);  // PRINT ADDRESS OF d, e, f
  d=4; e=5; f=6;
  B(d,e);
  printf("exit A\n");
}

int B(int x, int y)
{
  int g,h,i;
  printf("enter B\n");
  printf("address of g=%8x, h=%8x, i=%8x\n", &g, &h,&i);// PRINT ADDRESS OF g,h,i
  g=7; h=8; i=9;
  C(g,h);
  printf("exit B\n");
}

int C(int x, int y)
{
  int u, v, w, i, *p;

  printf("enter C\n");
  printf("address of u=%8x, v=%8x, w=%8x\n", &u, &v,&w);// PRINT ADDRESS OF u,v,w;
  u=10; v=11; w=12;

  asm("movl %ebp, FP");    // CPU's ebp register is the FP
  printf("FP = %8x\n", FP);
  printf("the content of stack:\n");
  int n;
  for (n=-8; n<110; n=n+1)
  {	
    printf("FP[%d]=%8x\n ", n, &FP[n]);
  }
  int asd;
  printf("u = %8x\n", asd = *&FP[-4]);
}
