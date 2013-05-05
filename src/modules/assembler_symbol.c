SYMBOL *symbol_add(List *symtab, char *query, NODE *link){
	if(query == NULL)
		return NULL;
	{
		SYMBOL *existed;
		if((existed	= symbol_search(symtab, query)) != NULL){
			if(link == NULL){
				if(0)
					printf("%s<use ptr> %p\n", existed->symbol, link);
			}else{
				if(0)
					printf("%s<set ptr> %p\n", existed->symbol, link);
				existed->link = link;
			}
			return existed;
		}else{
			SYMBOL *new = (SYMBOL *)calloc(1, sizeof(SYMBOL));
			new->symbol = (char *)calloc(7, sizeof(char));
			strcpy(new->symbol, query);
			if(link != NULL)
				new->link = link;
			if(0){
				if(link == NULL){
					printf("%s<new ptr> %p\n", new->symbol, link);
				}else{
					printf("%s<new one> %p\n", new->symbol, link);
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

bool symbol_less_func(const struct list_elem *a, const struct list_elem *b, void *aux){
	SYMBOL *sa = list_entry(a, SYMBOL, elem),
		   *sb = list_entry(b, SYMBOL, elem);
	return strcasecmp(sa->symbol, sb->symbol) <= 0;
}

void symbol_view(List *symtab){
	list_sort(symtab, symbol_less_func, NULL);
	Elem *find;
	for(find = list_begin(symtab);
		find != list_end(symtab);
		find = list_next(find)){
			SYMBOL *now = list_entry(find, SYMBOL, elem);
			printf("\t%s\t%04X\n", now->symbol, (unsigned int)now->link->LOCATION_CNT); 
	}
}
