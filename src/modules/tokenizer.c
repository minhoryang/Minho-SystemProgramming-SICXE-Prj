#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // bool
#include <string.h>  // memset

#include "core/macro.h"
#include "core/hex.h"
#include "modules/tokenizer.h"

#ifdef tokenizer_test
int main(){
	char *s, *o = "hello  @X'123' +world  T'   c'Ei OF' world\t.comment C'123'";

	Tokenizer(o);
	printf("\n<    %s\n", o);
	printf(">    %s\n", (s = Tokenizer_FindQuotes(o)));
	printf(">>   %s\n", (o = Tokenizer_NoComments(s))); free(s);
	printf(">>>  %s\n", (s = Tokenizer_DeBlanks(o))); free(o);
	printf(">>>> %s\n", (o = Tokenizer_NoSpecialChars(s))); free(s);
	free(o);

	return 0;
}
#endif

char *Tokenizer(char* const line){
	// XXX : Split words per Tokenizer_Separator.
	char *i, *j, //*part,
	     *result = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	size_t len_result = 0;
	bool is_found = false;

	// XXX : PIPELINE!
	j = Tokenizer_FindQuotes(line);
	i = Tokenizer_NoComments(j); free(j);
	j = Tokenizer_DeBlanks(i); free(i);
	i = Tokenizer_NoSpecialChars(j); free(j);

	// XXX : Split.
	for(j = i; *j != '\0'; j++){  // for every line.
		if(!is_found){
			if(*j != Tokenizer_Separator){
				is_found = true;  // part start + data
				result[len_result++] = *j;
			}else{} // ignored.
		}else{
			if(*j == Tokenizer_Separator){
				is_found = false;  // part end
				result[len_result++] = '\n';
			}else{
				result[len_result++] = *j;  // part data
			}
		}
	}

	return result;
}

char const Tokenizer_Allows[5] = {'#', '@', '.', '+', Tokenizer_Separator};
size_t const Tokenizer_Allows_Cnt = 5;
char const Tokenizer_Blanks[4] = {' ', '\t', '\n', '\r'};
size_t const Tokenizer_Blanks_Cnt = 4;

char *Tokenizer_FindQuotes(char* const line){
	// XXX : Find Quotes inside of line, and replace it.
	char *before, *now,  // current pointer @ line.
		 *result = (char *)calloc(Tokenizer_Max_Length, sizeof(char)),
		 *found = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	size_t len_found = 0,
		   len_result = 0;
	bool is_found = false,  // true, when currently handling quotes.
		 is_ignore_1_letter_only = false;
	enum { chars = 0, hexs } type_found;
	int _temp, quote = 39;

	for(before = (now = line) - 1;  // for every letters..
		(*now != '\0');
		before++, now++){

		if(!is_found){  // A. not found, then try to find.
			if(*now == quote){  // A-1. Find!!
				is_found = true;
				switch(*before){  // A-1-a. Get type of quotes.
					case 'X': case 'x':
						type_found = hexs;
						break;
					case 'C': case 'c':
						type_found = chars;
						break;
					default:  // A-1-b. Failed, then
						{  // <Reset flags>
							is_found = false;
							len_found = 0;
							memset(found, 0, Tokenizer_Max_Length);
						}
						{  // <Copy to result normally> <-- A-2-b.
							result[len_result++] = *before;
						}
						break;
				}
			}else{  // A-2. Failed to find, Try to Copy.
				if(line != now){  // (1 letter delayed.)
					if(is_ignore_1_letter_only){
						// A-2-a.  [ISSUE]
						//    After handling quotes, last quote printed.
                        //    Because we wrote as 1 letter delayed.
                        //    So after handling quotes, ignore 1 letter.
						is_ignore_1_letter_only = false;
					}else{  // A-2-b. Copy to result normally.
						result[len_result++] = *before;
					}
				}
			}
		}else{  // B. Found!, so try to get it.
			if(*now != quote){
				found[len_found++] = *now;  // B-1. DATA Area.
			}else{
				switch(type_found){  // B-2. Successfully got the data.
					case hexs:  // B-2-a. hex conveting and copying.
						//_ use _temp temporary.
						_temp = hex2int(found);
						//_ reuse *found for copying.
						sprintf(found, "%d", _temp);
						for(len_found = 0;
							len_found < len_int(_temp);
							len_found++){

							result[len_result++] = found[len_found];
						}
						break;
					case chars:  // B-2-b. char copying.
						//_ use _temp temporary.
						for(_temp = 0;
							_temp < len_found;
							_temp++){

							result[len_result++] = found[_temp];
						}
						break;
				}
				{  // <Reset Flags>
					len_found = 0;
					is_found = false;
					memset(found, 0, Tokenizer_Max_Length);
				}
				// [ISSUE] <-- A.2.a.
				is_ignore_1_letter_only = true;
			}
		}
	}
	{  // <Handling Last Letter>.
		if(is_ignore_1_letter_only){  // <-- A-2-a.
			is_ignore_1_letter_only = false;
		}else{  // <-- A-2-b.
			result[len_result++] = *before;
		}
	}
	free(found);
	return result;
}

char *Tokenizer_NoComments(char* const line){
	// XXX : Delete all chars between '.' and '/n'.
	char *now,
         *result = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	size_t len_result = 0;

	for(now = line; *now != '\0'; now++){  // for every letters..
		result[len_result++] = *now;  // copy it.
		if(*now == '.'){  // if find .dot, mission complete!
			result[len_result++] = Tokenizer_Separator;
			break;
		}
	}
	return result;
}

char *Tokenizer_DeBlanks(char* const line){
	// XXX : Replace all blanks to 1 Tokenizer_Separator.
	char *now,
		 *result = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	size_t len_result = 0;
	bool is_found = false;

	for(now = line; *now != '\0'; now++){  // for every letters.
		if(!is_found){
			// Foreach(Tokenizer_Blanks) == *now?
			{
				size_t cur;
				for(cur=0; cur<Tokenizer_Blanks_Cnt; cur++){
					if(Tokenizer_Blanks[cur] == *now){
						is_found = true;
						break;
					}
				}
			}
			// Handling result:
			if(!is_found){
				// failed to find. -> Copy!
				result[len_result++] = *now;
			}else{
				// found! -> Insert Separator!
				result[len_result++] = Tokenizer_Separator;
			}
		}else{
			bool is_found_again = false;
			// Try to find [Blanks]. Again.
			{
				size_t cur;
				for(cur=0; cur<Tokenizer_Blanks_Cnt; cur++){
					if(Tokenizer_Blanks[cur] == *now){
						is_found_again = true;
						break;
					}
				}
			}
			// Handling result:
			if(is_found_again){
				// ignore!
			}else{
				is_found = false;
				result[len_result++] = *now;
			}
		}
	}
	return result;
}

char *Tokenizer_NoSpecialChars(char * const line){
	// XXX : Remove all not-allowed letters @ line.
	char *now,
	     *result = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	size_t len_result = 0;

	for(now = line; *now != '\0'; now++){  // for every letters..
		bool is_allowed = false;

		// check if in alphabets.
		if(('A' <= *now) && (*now <= 'Z')){
			is_allowed = true;
		}else if(('a' <= *now) && (*now <= 'z')){
			is_allowed = true;
		}else if(('0' <= *now) && (*now <= '9')){  // check if in numbers.
			is_allowed = true;
		}else{  // check if in Tokenizer_Allows.
			size_t cur;
			for(cur=0; cur<Tokenizer_Allows_Cnt; cur++){
				if(Tokenizer_Allows[cur] == *now){
					is_allowed = true;
					break;
				}
			}
		}

		if(is_allowed){
			result[len_result++] = *now;
		}else{
			result[len_result++] = Tokenizer_Separator;
		}
	}
	return result;
}
