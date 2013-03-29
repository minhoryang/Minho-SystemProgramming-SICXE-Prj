#ifndef src_modules_tokenizer
	#define src_modules_tokenizer
	#define Tokenizer_Separator '!'
	#define Tokenizer_Quotes_Reservation '$'
	#define Tokenizer_Max_Length 100

	typedef struct {
		size_t cnt;
		char **parts;
	}Quotes;

	char *Tokenizer(char * const);
	char *Tokenizer_FindQuotes(char * const, Quotes *);
	char *Tokenizer_NoComments(char * const);
	char *Tokenizer_DeBlanks(char * const);
	char *Tokenizer_NoSpecialChars(char * const line);
	char *Tokenizer_FillQuotes(char * const, Quotes *);

	Quotes *AllocQuotes();
	void DeAllocQuotes(Quotes *in);
#endif
