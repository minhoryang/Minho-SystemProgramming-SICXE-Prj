#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "modules/history.h"
#include "modules/tokenizer.h"

#ifdef history_test
	int main(){
		return 0;
	}
#endif

void History_Add(Environment *env){
	History *new = (History *)calloc(1, sizeof(History));
	new->line = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	strncpy(new->line, env->line, strlen(env->line));
	list_push_back(env->history, &new->elem);
}

void History_View(List *h){
	struct list_elem *find;
	size_t i = 1;
	for(find = list_begin(h);
		find != list_end(h);
		find = list_next(find)){
			History *old = list_entry(find, History, elem);
			printf("%lu\t%s", i++, old->line);
	}
}

void History_DeAlloc(List *h){
	struct list_elem *find;
	for(find = list_begin(h);
		find != list_end(h);
		/* do nothing */){
			History *old = list_entry(find, History, elem);  // tada.
			find = list_next(find);  // go farther before deleted.
			free(old->line);
			free(old);  // DeAllocation
	}
}
