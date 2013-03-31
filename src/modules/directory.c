#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // strcmp().
#include <dirent.h>  // struct dirent, scandir().
#include <sys/stat.h>  // lstat(), S_ISDIR(), S_IXUER.

#include "modules/directory.h"

#ifdef directory_test
	int main(){
	    struct dirent **items; 
		int i, enter,  // loop variables.
			nitems = scandir(".", &items, NULL, alphasort);  // Get the list of alphabet-sorted elements in 'current directory' at **items. 

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

		free(items);
		return 0;
	}
#endif


