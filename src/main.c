#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "core/argument.h"
#include "modules/tokenizer.h"

int main(int argc, char *argv[]){
	FILE *fp = argument_file_opener(argc, argv);  // open file.
	char *line = (char *)calloc(Tokenizer_Max_Length, sizeof(char)), *out;

	while(fgets(line, Tokenizer_Max_Length, fp) != NULL){  // for every inputed lines,
		printf("%s", (out = Tokenizer(line))); free(out);  // Tokenizer works perfectly.
	}

	free(line);  // cleanup.
	fclose(fp);
	return 0;
}
