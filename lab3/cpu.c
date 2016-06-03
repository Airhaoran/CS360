#include <stdio.h>
#include <stdlib.h>
main()
{
   int ebp, esp;
   ebp = get_ebp();
   esp = get_esp();
   printf("ebp=%8x   esp=%8x\n", ebp, esp);
}
