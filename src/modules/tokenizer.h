#ifndef src_modules_tokenizer
	#define src_modules_tokenizer
	#define Tokenizer_Separator '!'
	#define Tokenizer_Max_Length 100

	char *Tokenizer(char * const);
	char *Tokenizer_FindQuotes(char * const);
	char *Tokenizer_NoComments(char * const);
	char *Tokenizer_DeBlanks(char * const);
	char *Tokenizer_NoSpecialChars(char * const line);
#endif
