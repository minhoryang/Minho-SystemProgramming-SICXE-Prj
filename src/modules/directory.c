#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // strcmp().
#include <dirent.h>  // struct dirent, scandir().
#include <sys/stat.h>  // lstat(), S_ISDIR(), S_IXUER.

#include "modules/directory.h"

#ifdef directory_test
	int main(){
		Directory_Print(".");
		//Directory_File_Type("test.txt");
		return 0;
	}
#endif

void Directory_Print(char * const directory){
	struct dirent **items = NULL; 
	int i, enter,  // loop variables.
		nitems = scandir(directory, &items, NULL, alphasort);  // Get the list of alphabet-sorted elements in *directory to **items. 

	for (i = 0, enter = 0; i < nitems; i++, enter++){  // foreach **items:
		// Ignore hidden files, dirs.
		if(items[i]->d_name[0] ==  '.'){
			enter--;
			continue;
		}

		// Print name:
		printf("%s", items[i]->d_name);

		// Check whether directory or executable file:
		{
			struct stat mystat;
			lstat(items[i]->d_name, &mystat);  // Get the status of 'current element'.
			if(S_ISDIR(mystat.st_mode))
				printf("/");
			else if((mystat.st_mode & S_IXUSR))
				printf("*");
		}

		// Print 'divide char':
		if((enter + 1)%3)
			printf("\t");
		else
			printf("\n");
	}

	if((enter)%3)
		printf("\n");

	if(items != NULL)
		free(items);
}

void Directory_File_Type(char * const file){
	FILE *fp = fopen(file, "r");
	if(fp == NULL){
		printf("err!\n");
		return ;
	}
	int b, ch;
	for(b = '\n'; (ch = getc(fp)) != EOF; b = ch)
		putchar(b);
	putchar(b);
	fclose(fp);
}
