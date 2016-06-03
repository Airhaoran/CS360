#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct node {
    struct node *siblingPtr, *childPtr, *parentPtr;
    char type;
    char name[64];
} node;

node *root, *pwd;
char input[128], command[64], pathname[64], dirname[64], basename[64];
FILE *fp;


int findCommand(char com[64]);
void init(void);
node *mkdir(char *n, char t, node *cwd);
node *cd(char *n, node *cwd);
void ls(node *cwd);
char *pwdir(node *cwd);
int rmdir(char n[64]);
void rm(node *child);
void readinputplex(void);
void save(char *n, node *cwd);
void reverse(char *c);
void create(char c[128]);

int main() {
    char temp[128];
    node *tempPtr;
    init();
    while(1) {

        printf("user@shell$");
        //pwdir(pwd);
        //printf(" $ ");

        readinputplex();

        switch (findCommand(command)) {
        case 0:
            strcpy(temp,basename);
            strcat(temp,dirname);
            mkdir(temp, 'D',pwd);
            break;
        case 1:
            ls(pwd->childPtr);
            break;
        case 2:
			if(strchr(pathname,'/')!=0&&root->childPtr!=0){
				pwd = cd(pathname,root->childPtr);
				break;
			}
            pwd = cd(basename,pwd->childPtr);
            break;
        case 3:
            pwdir(pwd);
            break;
        case 4:
            rmdir(basename);
            break;
        case 5:
            fp = fopen(basename, "w+");
            save(basename,root);
            fclose(fp);
            break;
		case 6:
			if(strchr(pathname,'/')==NULL){// / not found
				create(basename);
				break;
			}
			create(pathname);
			break;
		case 7:
			rmdir(basename);
			break;
        case 9:
            return 0;
            break;

        default:
            break;
        }

    }

    return EXIT_SUCCESS;
}

void init(void) {
    char r[10] = "/";
    root = (node *)malloc(sizeof(node));
    root->type='D';
    strcpy(root->name,r);
    root->siblingPtr=0;
    root->childPtr=0;
    root->parentPtr=0;
    pwd = root;
}

node *mkdir(char *n, char t, node *cwd) {
    if(strchr(n,'/')==NULL) { // if there is a NOT / in the input
        node *child;
        child = (node *)malloc(sizeof(node));
        strncpy(child->name,n,64);
        child->type=t;
        child->siblingPtr=cwd->childPtr;
        child->childPtr=0;
        child->parentPtr=pwd;
        cwd->childPtr = child;
        return pwd->childPtr;
    }

    // if there IS a / in the input

    char *token;
    node *temp;
    token = strtok(pathname,"/");
    while(token!=NULL) {
        printf("Token: %s \n",token);
        if(strcmp(token,basename)==0) {
            mkdir(token,t,cwd);
            return cwd;
        }
        cwd = cd(token,cwd->childPtr);
        if(cwd==0||cwd==-1) {
            printf("Directory doesnt exist\n");
            return cwd;
        }
        token =strtok(NULL, "/\n");
    }
    mkdir(basename,t,cwd);
    return cwd;

}

node *cd(char *n,node *cwd) {
    if(*n==0|| strcmp(n,"/")==0) {
        cwd=root;
        return cwd;
    }
    if(strchr(n,'/')!=NULL) { //if there is a /
        char *token;
        token = strtok(n,"/");
        while(token!=NULL&&cwd!=0) {
            while(cwd!=0) {
                if(cwd->type!='D') {
                    printf("Error not a directory!\n");
                    return pwd;
                }
                if(strncmp(cwd->name,token,64)==0) {
                     cwd=cwd->childPtr;
                     break;
                } else {
                    cwd = cwd->siblingPtr;
                }
            }
            token=strtok(NULL,"/\n");
		}
		if(cwd!=root)
			return cwd->parentPtr;
		return pwd;
	}


    while(cwd!=0) {		// if there is not a /
        if(cwd->type!='D') {
            printf("Error not a directory!\n");
            break;
        }
        if(strncmp(cwd->name,n,64)==0) {
            return cwd;
        } else {
            cwd = cwd->siblingPtr;
        }
    }
    printf("Error directory not found\n");
    return pwd;

}

void ls(node *cwd) {
	if(strchr(pathname,'/')==NULL) { //if there is not a /
		if(cwd==0)
			return;
		while(cwd != 0) {
			printf("%s ", cwd->name);
			cwd = cwd->siblingPtr;
		}
		printf("\n");
		return;
	}
	char *token;
	token = strtok(pathname,"/\n");
	cwd = root;
	while(token!=NULL&&cwd!=-1) {
		cwd = cd(token,cwd->childPtr);
		token=strtok(NULL,"/\n");
	}
	memset(&pathname[0],0,sizeof(pathname));
	if(cwd!=-1 && cwd->childPtr!=0){
		ls(cwd->childPtr);
		return;
	}
	printf("Directory not found\n");
	return;
}

int findCommand(char com[64]) {
    if(!strncmp(com,"mkdir",64)) {
        //printf("MAKEDRIVE\n");
        return 0;
    }
    if(strncmp(com,"ls",64)==0) {
        //printf("LS\n");
        return 1;
    }
    if(strncmp(com,"cd",64)==0) {
        //printf("CD\n");
        return 2;
    }
    if(strncmp(com,"pwd",64)==0) {
        //printf("PWD\n");
        return 3;
    }
    if(strncmp(com,"rmdir",64)==0) {
        return 4;
    }
    if(strncmp(com,"save",64)==0) {
        return 5;
    }
    if(strncmp(com,"create",64)==0){
    	return 6;
    }
    if(strncmp(com,"rm",64)==0){
    	return 7;
    }
    if(strncmp(com,"?",64)==0) {
        printf("The commands are: mkdir ls rmdir cd pwd rmdir save create ? \n");
        return 8;
    }
    if(strncmp(com,"q",64)==0 ||strncmp(com,"quit",64)==0 ||strncmp(com,"exit",64)==0) {
        printf("QUIT\n");
        return 9;
    }

    printf("Error command not found type ? for help\n");
    return -1;

}

char *pwdir(node *cwd) {

    char c[128] ="";
    if(pwd==root)strcat(c,"/");
    while(cwd !=root) {

        strcat(c,cwd->name);
        strcat(c,"/");
        cwd = cwd->parentPtr;
    }

//	strrev(c);
    reverse(c);
    printf("%s\n",c);
    return c;
}

void reverse(char *c) {
    int len = strlen(c);
    int h, i, j;
    for(i=0, j=len-1; i<j; i++,j--) {
        h = c[i];
        c[i]=c[j];
        c[j]=h;
    }
}

int rmdir(char n[64]) {
    if(pwd->childPtr==0) {
        printf("Cannot find that directory\n");
        return -1;
    }
    node * cwd = pwd->childPtr;
    if(strncmp(cwd->name,n,64)==0) { //if the first value is the rmdir
        pwd->childPtr=cwd->siblingPtr;
        free(cwd->childPtr);
        return 0;
    }
    while(cwd->siblingPtr!=0) {		// check to see if no more siblings
        if(strncmp(cwd->siblingPtr->name,n,64)==0) {
            node *temp;
            temp = cwd->siblingPtr->siblingPtr;
            free(cwd->siblingPtr);
            cwd->siblingPtr=temp;
            if(temp!=0)
                rm(temp->childPtr);
            free(temp);
            return 0;
        }
        cwd=cwd->siblingPtr;
    }
    printf("Cannot find that directory\n");
    return -1;
}

void rm(node *child) {
    if(child!=0) {
        rm(child->siblingPtr);
        rm(child->childPtr);
        free(child);
    }
}

void readinputplex() {
    char *token, *dir;
    memset(&input[0],0,sizeof(input));
    memset(&pathname[0],0,sizeof(pathname));
    memset(&dirname[0],0,sizeof(dirname));
    memset(&basename[0],0,sizeof(basename));


    fgets(input, 128, stdin);
    token = strtok(input, " \n/");
    if(token!=NULL) {
        strcpy(command,token);
    }
    token = strtok(NULL, " \n");
    if(memchr(input,'/',128)!=NULL) { // if there is a / in the input
        if(token!=NULL) dir = strrchr(token,'/')+1;
        if(dir!=NULL && token!=NULL) {
            strcpy(basename, dir);
        }
        while(token!=NULL) { // TODO: change dirname to dirname - basename
            strcpy(pathname,token);
            strncpy(dirname,pathname,strlen(pathname)-strlen(basename));
            token = strtok(NULL, "\n");
        }
        printf("Pathname: %s\n",pathname);
        printf("Dirname: %s\n",dirname);
        printf("Basename: %s\n",basename);

        return;
    }
    if(token!=NULL) {
        strcpy(basename, token);
    }
    return;
}

void save(char *filename, node *cwd) {
    if(cwd!=0) {
    	fputs(&cwd->type,fp);
    	fputs(" ",fp);
    	fputs(pwdir(cwd),fp);
    }
    if(cwd->siblingPtr!=0) {
		fputs("\n",fp);
        save(filename, cwd->siblingPtr);
        return;
    }
    if(cwd->childPtr!=0) {
        fputs("\n",fp);
        save(filename, cwd->childPtr);
    }
return;


}

void create(char c[128]){
	mkdir(c, 'F', pwd);
}
/*eof*/
