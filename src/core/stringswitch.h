#ifndef src_core_stringswitch
	#define src_core_stringswitch

	typedef struct {
		char **list;
		size_t length;
	}StringSwitchSet;

	StringSwitchSet *AllocStringSwitchSet(char *[], size_t);
	void DeAllocStringSwitchSet(StringSwitchSet *);
	int String2Switch(StringSwitchSet *, char * const);

	#define StringSwitch(a, b) switch(String2Switch((a), (b)))
#endif
