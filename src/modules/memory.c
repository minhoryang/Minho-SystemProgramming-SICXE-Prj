#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // memset().
#include <stdbool.h>  // bool.
#include "core/hex.h"
#include "modules/memory.h"

#ifdef memory_test
int main(){
	unsigned char *memory = (unsigned char *)calloc(MEM_MAX, sizeof(unsigned char));
	size_t prev = 0;
	memory_edit(memory, 4, hex2int("6D"));
	memory_fill(memory, hex2int("24"), hex2int("34"), hex2int("2A"));
	prev += memory_dump(memory, prev, prev + TEN_LINES);  // 'dump' : start:[$prev(=0)] to:[$prev + "9F"] (10 lines).
	prev += 1;
	prev += memory_dump(memory, prev, prev + TEN_LINES);  // 'dump' : start:[$prev(=0)] to:[$prev + "9F"] (10 lines).
	memory_reset(memory);
	free(memory);
	return 0;
}
#endif

size_t memory_dump(void *memory, size_t start, size_t end){
	// XXX : Dump *memory from 'start' to 'end'.

	// Validating start-end.
	if((start < MEM_MIN) || (MEM_MAX < end))
		return 0;

	// Printing those things:
	size_t base, x;
	for(base = start & FFFF0_8; base <= end; base+=16){
		// A. Base Address.
		printf("%05X ", (int)base);
		// B. Hex Data.
		for(x=base; x<base+16; x++){
			if(!(x < start) && !(end < x))  // Check if 'x' is between start and end.
				printf(" %02X", *(unsigned char *)(memory + x));
			else
				printf("   ");
		}
		// C. Dividing columns.
		printf(" ; ");
		// D. ASCII code.
		for(x=base; x<base+16; x++){
			if(!(x < start) && !(end < x)){  // Check if 'x' is between start and end.
				if((32 <= *(unsigned char *)(memory + x)) && (*(unsigned char *)(memory + x) <= 126))  // Check if is Printable.
					printf("%c", *(unsigned char *)(memory + x));
				else
					printf(".");
			}else
				printf(".");
		}
		// E. Line end.
		printf("\n");
	}

	return end;
}

bool memory_edit(void *memory, size_t address, size_t value){
	if((address < MEM_MIN) || (MEM_MAX < address))
		return false;
	*(unsigned char *)(memory+address) = value;
	return true;
}

bool memory_fill(void *memory, size_t start, size_t end, size_t value){
	// Validating start-end.
	if((start < MEM_MIN) || (MEM_MAX < end))
		return false;

	size_t x;
	for(x=start; x<=end; x++)
		if(!memory_edit(memory, x, value))
			return false;
	return true;	
}

void memory_reset(void *memory){
	memset(memory, 0, MEM_MAX * sizeof(unsigned char));
}
