SYMBOL *symbol_add(List *symtab, char *query, NODE *link, bool TO_SAVE){
	if(query == NULL)
		return NULL;
	{
		SYMBOL *existed;
		if((existed	= symbol_search(symtab, query)) != NULL){
			if(!TO_SAVE){
				if(0)
					printf("%s<use>\t", existed->symbol);
			}else{
				if(0)
					printf("%s<old>\t", existed->symbol);
				existed->SAVED = TO_SAVE;
				existed->link = link;
			}
			return existed;
		}else{
			SYMBOL *new = (SYMBOL *)calloc(1, sizeof(SYMBOL));
			new->symbol = (char *)calloc(7, sizeof(char));
			strcpy(new->symbol, query);
			new->link = link;
			new->SAVED = TO_SAVE;
			if(0){
				if(!TO_SAVE){
					printf("%s<ptr>\t", new->symbol);
				}else{
					printf("%s<save>\t", new->symbol);
				}
			}
			list_push_back(symtab, &(new->elem));
			return new;
		}
	}
}

SYMBOL *symbol_search(List *symtab, char *query){
	Elem *find;
	for(find = list_begin(symtab);
		find != list_end(symtab);
		find = list_next(find)){
			SYMBOL *now = list_entry(find, SYMBOL, elem);
			if(strcasecmp(now->symbol, query) == 0)
				return now;
	}
	return NULL;
}

