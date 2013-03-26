#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "core/argument.h"
#include "modules/tokenizer.h"

int main(int argc, char *argv[]){
	FILE *fin = argument_file_opener(argc, argv), *fout;  // open file.
	char *line = (char *)calloc(Tokenizer_Max_Length, sizeof(char)), *out;

    if(fin == stdin)
		fout = stdout;
	else
		fout = fopen("output.txt", "w");

	while(fgets(line, Tokenizer_Max_Length, fin) != NULL){  // for every inputed lines,
		fprintf(fout, "%s", (out = Tokenizer(line))); free(out);  // Tokenizer works perfectly.
	}

	free(line);  // cleanup.
	fclose(fin);
	fclose(fout);
	return 0;
}
