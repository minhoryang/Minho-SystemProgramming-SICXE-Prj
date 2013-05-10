#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // bool
#include <string.h>  // memset

#include "core/macro.h"
#include "core/hex.h"
#include "modules/tokenizer.h"

#ifdef tokenizer_test
	#include "core/argument.h"
	#define src_modules_tokenizer_ignore_changes_for_assembler

	int main(int argc, char *argv[]){
		// The main function for Test-driven development (TDD).
		FILE *fin = argument_file_opener(argc, argv),  // open file.
			 *fout = fopen("output.txt", "w");
	#ifndef src_modules_tokenizer_deprecated
		char *line = (char *)calloc(Tokenizer_Max_Length, sizeof(char)), **out;
		size_t i, cnt;
	#else
		char *line = (char *)calloc(Tokenizer_Max_Length, sizeof(char)), *out;
	#endif
	
		while(fgets(line, Tokenizer_Max_Length, fin) != NULL){  // for every inputed lines,
	#ifndef src_modules_tokenizer_deprecated
			cnt = Tokenizer(line, (out = AllocToken()), SIC);  // Tokenizer works perfectly.
			for(i = 0; i < cnt; i++){
				fprintf(fout, "%s\n", out[i]);
				printf("%s\n", out[i]);
			}
			DeAllocToken(out);
	#else
			fprintf(fout, "%s", (out = Tokenizer(line)));  // Tokenizer works perfectly.
			printf("%s", out);
			free(out);
	#endif
		}
	
		free(line);  // cleanup.
		fclose(fin);
		fclose(fout);
		return 0;
	}
#endif

#ifndef src_modules_tokenizer_deprecated
	size_t Tokenizer(char* const line, char **result, ThreeStatesSelector flag){
		// XXX : Split words per Tokenizer_Separator.
		char *i, *j;  // PIPELINE variables.
		size_t len_found = 0,
			   len_result = 0;
		bool is_found = false;

		if(flag == SIC){
			// XXX : PIPELINE!
			Quotes *datas = AllocQuotes();
			j = Tokenizer_FindQuotes(line, datas);
			i = Tokenizer_NoComments(j); free(j);
			j = Tokenizer_DeBlanks(i); free(i);
			i = Tokenizer_SICXE(j); free(j);
			j = Tokenizer_FillQuotes(i, datas); free(i);
			i = j;
			DeAllocQuotes(datas);
		}else if(flag == NONE){
			i = Tokenizer_DeBlanks(line);
		}

		// XXX : Split.
		for(j = i; *j != '\0'; j++){  // for every line.
			if(!is_found){
				if(*j != Tokenizer_Separator){
					is_found = true;  // part start + data
					result[len_result][len_found++] = *j;
				}else{}  // ignored.
			}else{
				if(*j == Tokenizer_Separator){
					result[len_result][len_found++] = 0;
					{
						is_found = false;  // part end
						len_result++;
						len_found = 0;
					}
				}else{
					result[len_result][len_found++] = *j;
				}
			}
		}

		free(i);
		return len_result;
	}

	char **AllocToken(){
		char **token = (char **)calloc(Tokenizer_Max_Length, sizeof(char *));
		size_t i;
		for(i=0; i<Tokenizer_Max_Length; i++)
			token[i] = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
		return token;
	}

	void DeAllocToken(char **token){
		size_t i;
		for(i=0; i<Tokenizer_Max_Length; i++)
			free(token[i]);
		free(token);
	}
#else
	char *Tokenizer(char* const line){
		// XXX : Split words per Tokenizer_Separator.
		char *i, *j, //*part,
		     *result = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
		size_t len_result = 0;
		bool is_found = false;

		// XXX : PIPELINE!
		Quotes *datas = AllocQuotes();
		j = Tokenizer_FindQuotes(line, datas);
		i = Tokenizer_NoComments(j); free(j);
		j = Tokenizer_DeBlanks(i); free(i);
		i = Tokenizer_FillQuotes(j, datas); free(j);
		DeAllocQuotes(datas);
	
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
#endif

char const Tokenizer_Blanks[5] = {' ', '\t', '\n', '\r', ','};
size_t const Tokenizer_Blanks_Cnt = 5;

char *Tokenizer_FindQuotes(char* const line, Quotes *datas){
	// XXX : Find Quotes inside of line, and Store that data to *datas.
	char *before, *now,  // current pointer @ line.
		 *result = (char *)calloc(Tokenizer_Max_Length, sizeof(char)),
		 *found;
	size_t len_found = 0,
		   len_result = 0;
	bool is_found = false,  // true, when currently handling quotes.
		 is_ignore_1_letter_only = false;
	enum { chars = 0, hexs } type_found;

	for(before = (now = line) - 1;  // for every letters..
		(*now != '\0');
		before++, now++){

		if(!is_found){  // A. not found, then try to find.
			if(*now == '\''){  // A-1. Find!!
				is_found = true;
				found = (char *)calloc(Tokenizer_Max_Length, sizeof(char)); 
				switch(*before){  // A-1-a. Get type of quotes.
					case 'X': case 'x':
						type_found = hexs;
						{  // XXX [FIXED] 'X' have to print also.
							//result[len_result++] = *before;  // XXX [FIXED][AGAIN] 'X' no need to print!
							result[len_result++] = Tokenizer_Separator;
						}
						break;
					case 'C': case 'c':
						type_found = chars;
						{  // XXX [FIXED] 'C' have to print also.
							//result[len_result++] = *before;  // XXX [FIXED][AGAIN] 'C' no need to print!
							result[len_result++] = Tokenizer_Separator;
						}
						break;
					default:  // A-1-b. Failed, then
						{  // <Reset flags>
							is_found = false;
							len_found = 0;
							free(found);
							found = NULL;
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
			if(*now != '\''){
				found[len_found++] = *now;  // B-1. DATA Area.
			}else{
#ifndef src_modules_tokenizer_ignore_changes_for_assembler
				char *new;
				size_t len_new, l;
#endif
				switch(type_found){  // B-2. Successfully got the data.
					case hexs:  // B-2-a. for hex: converting.
#ifdef src_modules_tokenizer_ignore_changes_for_assembler
						sprintf(found, "%d", hex2int(found));
#endif
						break;
					case chars:  // B-2-b. for char.
#ifndef src_modules_tokenizer_ignore_changes_for_assembler
						new = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
						len_new = strlen(found);
						for(l=0;l<len_new;l++)
							sprintf(new + l * 2, "%02X", found[l]);
						free(found);
						found = new;
#endif
						break;
				}
				// B-2-c. Store 'found' data to '(Quotes *)datas'!
				{
					datas->parts[datas->cnt] = found;
					{
						char *itoa = (char *)calloc(10, sizeof(char));
						int len;
						sprintf(itoa, "%c%lu", '$', (datas->cnt)++);  // same as 'itoa()'.
						for(len = 0; len < strlen(itoa); len++){
							result[len_result++] = itoa[len];  // B-2-d. Reserved it.
						}
						result[len_result++] = Tokenizer_Separator;
						free(itoa);
					}
				}
				{  // <Reset Flags>
					len_found = 0;
					is_found = false;
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
	return result;
}

char *Tokenizer_NoComments(char* const line){
	// XXX : Delete all chars between '.' and '/n'.
	char *now,
         *result = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	size_t len_result = 0;

	for(now = line; *now != '\0'; now++){  // for every letters..
		if(*now == '.'){  // if find .dot, mission complete!
			result[len_result++] = Tokenizer_Separator;
			result[len_result++] = *now;  // safely copy it.
			result[len_result++] = Tokenizer_Separator;
			break;
		}else{
			result[len_result++] = *now;  // copy it.
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
	result[len_result++] = Tokenizer_Separator;  // XXX [PATCH] When 1 words entered, cnt=0. but should be cnt=1. 
	return result;
}

char *Tokenizer_SICXE(char * const line){
	// XXX : Helper for SIC/XE.
	char *now,
	     *result = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	size_t len_result = 0;

	for(now = line; *now != '\0'; now++){  // for every letters..
		switch(*now){
			case '@':
			case '#':
				result[len_result++] = *now;
				result[len_result++] = Tokenizer_Separator;
				break;
			default:
				result[len_result++] = *now;
		}
	}
	return result;
}

char *Tokenizer_FillQuotes(char * const line, Quotes *datas){
	char *now,
		 *result = (char *)calloc(Tokenizer_Max_Length, sizeof(char)),
		 *filter = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	size_t len_result = 0;

	sprintf(filter, "%c%%d%c", Tokenizer_Quotes_Reservation, Tokenizer_Separator);

	for(now = line; *now != '\0'; now++){
		if(*now == Tokenizer_Quotes_Reservation){
			size_t cnt = 0;
			sscanf(now, filter, &cnt);  // 
			//printf("<%lu>", cnt);
			{
				char *itoa = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
				sprintf(itoa, "%c%lu", Tokenizer_Quotes_Reservation, cnt); 
				now += strlen(itoa);  // Jump to outside of Quotes.
				free(itoa);
			}
			{
				size_t s;
				for(s=0;s<strlen(datas->parts[cnt]);s++){
					result[len_result++] = datas->parts[cnt][s];
				}
				result[len_result++] = Tokenizer_Separator;  // XXX FIXED) EOF missing.
			}
			
		}else{
			//printf("%c", *now);
			result[len_result++] = *now;
		}
	}

	free(filter);
	return result;
}

char *Tokenizer_NoEnter(char* const line){
	char *now,
         *result = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	size_t len_result = 0;

	for(now = line; *now != '\0'; now++){  // for every letters..
		if((*now != '\n') && (*now != '\r')){
			result[len_result++] = *now;  // safely copy it.
		}
	}
	return result;
}


Quotes *AllocQuotes(){  // Allocate Quote DB.
	Quotes *s = (Quotes *)calloc(1, sizeof(Quotes));
	s->cnt = 0;
	s->parts = (char **)calloc(Tokenizer_Max_Length, sizeof(char *));
	return s;
}

void DeAllocQuotes(Quotes *in){  // Free Quote DB.
	size_t i = 0;
	for(i=0;i<in->cnt;i++){
		free((in->parts)[i]);
	}
	free(in->parts);
	free(in);
	return;
}
