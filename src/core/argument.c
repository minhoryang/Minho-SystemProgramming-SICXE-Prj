#include <stdio.h>

#ifdef argument_test
int main(int argc, char *argv[]){
	argument_file_opener(argc, argv);
	return 0;
}
#endif

FILE *argument_file_opener(int argc, char *argv[]){
	// XXX : Support both way for listening datas. PIPE & ARGUMENT.
	// TODO : ADD FLAGS for DEBUG/TEST/.
	FILE *fp = stdin;
	if(argc>1){
		fp = fopen(argv[1], "r");
	}
	return fp;
}
