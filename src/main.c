#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "main.h"
#include "core/argument.h"
#include "core/debug.h"
#include "modules/tokenizer.h"

int main(int argc, char *argv[]){
	/*
	FILE *fin = argument_file_opener(argc, argv),  // open file.
		 *fout = fopen("output.txt", "w");
	char *line = (char *)calloc(Tokenizer_Max_Length, sizeof(char)), *out;

	while(fgets(line, Tokenizer_Max_Length, fin) != NULL){  // for every inputed lines,
		fprintf(fout, "%s", (out = Tokenizer(line)));  // Tokenizer works perfectly.
		printf("%s", out);
		free(out);
	}

	free(line);  // cleanup.
	fclose(fin);
	fclose(fout);
	*/
	return 0;
}
