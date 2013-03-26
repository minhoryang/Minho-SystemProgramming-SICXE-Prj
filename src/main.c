#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "modules/tokenizer.h"

int main(){
	char *line = (char *)calloc(Tokenizer_Max_Length, sizeof(char)),
		 *o;
	FILE *fp = stdin;
	while(fgets(line, Tokenizer_Max_Length, fp)!=NULL){
		printf("%s", (o = Tokenizer(line))); free(o);
	}
	free(line);
	return 0;
}
