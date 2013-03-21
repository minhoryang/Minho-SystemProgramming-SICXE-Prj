#ifndef src_modules_tokenizer
	#define src_modules_tokenizer 
	char *Tokenizer_NoComments(char* const input);
	
	char const Tokenizer_Allows[3] = {'#', 39/*'*/, ','};
	char const Tokenizer_Blanks[2] = {' ', '\t'};
#endif
