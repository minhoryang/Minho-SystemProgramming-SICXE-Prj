#include <stdio.h>
#include <stdlib.h>
#include "core/hex.h"

#ifdef hex_test
int main(){
	return 0;
}
#endif

int hex2int(char* const hex){
	int result;
	sscanf(hex, "%x", &result);
	return result;
}

size_t len_int(int in){
	size_t len = 0;
	if(in>0) len++;
	while(in/10){
		len++;
		in/=10;
	}
	return len;
}
