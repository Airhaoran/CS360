
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>


int main()
{
	char sent[1000];
	char ans[1000];
	sprintf(sent,"%s ", "hello");
	strcpy(ans, sent);
	sprintf(sent, "%s", "world");
	strcat(ans,sent);
	printf("%s\n",ans );


}
