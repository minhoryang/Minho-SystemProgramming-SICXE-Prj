#ifndef src_modules_memory
	#define src_modules_memory

	#define MEM_MIN 0
	#define MEM_MAX 16*16*16*16*16

	#define FFFF0_8 1048560
	#define TEN_LINES hex2int("9F")  // needs "core/hex.h".

	size_t memory_dump(void *memory, size_t start, size_t end);
	bool memory_edit(void *memory, size_t address, int value);
	bool memory_fill(void *memory, size_t start, size_t end, int value);
	void memory_reset(void *memory);
#endif
