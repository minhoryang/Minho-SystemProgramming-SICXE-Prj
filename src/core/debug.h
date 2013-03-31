#ifndef src_core_debug
	#define src_core_debug
	#ifdef DEBUG
		#undef NDEBUG
	#else
		#define NDEBUG
	#endif

	#include <assert.h>

#endif
