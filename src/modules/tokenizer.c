#include <stdio.h>
#include <stdlib.h>

#include "core/macro.h"
#include "modules/tokenizer.h"

#ifdef tokenizer_test
int main(){
	return 0;
}
#endif

char *Tokenizer_NoComments(char* const input){
	// TODO : Delete all chars between '.' and '/n'.
	return NULL;
}

char *Tokenizer_SpaceNormalize(char* const input){
	// TODO : Replace all blanks to 1 space.
	return NULL;
}
