#ifndef src_modules_tokenizer
	#define src_modules_tokenizer
	#define Tokenizer_Separator 13  // XXX [FIXED] '!' will fail, when input includes this. Replace to normally un used char : CarrageReturn.
	#define Tokenizer_Quotes_Reservation '$'
	#define Tokenizer_Max_Length 100

	typedef struct {
		size_t cnt;
		char **parts;
	}Quotes;

	#ifndef src_modules_tokenizer_deprecated
		typedef	enum ThreeStatesSelector {
			NONE=0,
			SIC,
			SICXE
		}ThreeStatesSelector;

		size_t Tokenizer(char* const line, char **result, ThreeStatesSelector flag);
		char **AllocToken();
		void DeAllocToken(char **token);
	#else
		char *Tokenizer(char * const);
	#endif

	char *Tokenizer_FindQuotes(char * const, Quotes *);
	char *Tokenizer_NoComments(char * const);
	char *Tokenizer_DeBlanks(char * const);
	char *Tokenizer_NoSpecialChars(char * const line);
	char *Tokenizer_FillQuotes(char * const, Quotes *);
	char *Tokenizer_NoEnter(char* const line);

	Quotes *AllocQuotes();
	void DeAllocQuotes(Quotes *in);
#endif
