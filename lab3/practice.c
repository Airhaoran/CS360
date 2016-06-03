#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main( ){
	int number, factory;
	printf("Hello world\n");
	printf("Enter a number \n");
	scanf("%d", &number);
	printf("number = %d \n",number );
	factory = myFactory(number);
	printf("factory of %d is %d \n",number, factory );
	return 0;

}
// int sum(struct node *list)
//      {
//         return (list==0)? 0 : list->value + sum(list->next);
//      }


int myFactory(int number)
{
	//int result;
	return (number==1) ?   1 :  number * myFactory(number-1);
	//return result
}