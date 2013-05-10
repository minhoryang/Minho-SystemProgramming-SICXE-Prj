#include <stdio.h>
#include "core/argument.h"

#ifdef argument_test
int main(int argc, char *argv[]){
	FILE *fp = argument_file_opener(argc, argv);
	fclose(fp);
	return 0;
}
#endif

FILE *argument_file_opener(int argc, char *argv[]){
	// XXX : Support both way for listening datas. PIPE & ARGUMENT.
	// TODO : ADD FLAGS for DEBUG/TEST/.
	FILE *fp = stdin;
	if(argc>1){
		fp = fopen(argv[1], "r");
		if(fp == NULL){
			printf("Couldn't find that file.\n");
			exit(1);
		}
	}
	return fp;
}
