
CSECT *csect_alloc(){
	CSECT *new = (CSECT *)calloc(1, sizeof(CSECT));
	new->symtab = (List *)calloc(1, sizeof(List));
	list_init(new->symtab);
	new->datas = (List *)calloc(1, sizeof(List));
	list_init(new->datas);
	new->modtab = (List *)calloc(1, sizeof(List));
	list_init(new->modtab);
	new->littab = (List *)calloc(1, sizeof(List));
	list_init(new->littab);
	new->blocks = (List *)calloc(1, sizeof(List));
	list_init(new->blocks);
	new->filename = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	return new;
}

void csect_dealloc(CSECT *csect){
	struct list_elem *find;

	for(find = list_begin(csect->symtab);
		find != list_end(csect->symtab);
		/* Do Nothing */){
			SYMBOL *s = list_entry(find, SYMBOL, elem);
			find = list_next(find);
			if(s->symbol)
				free(s->symbol);
			free(s);
	}
	free(csect->symtab);

	for(find = list_begin(csect->datas);
		find != list_end(csect->datas);
		/* Do Nothing */){
			DATA *s = list_entry(find, DATA, elem);
			find = list_next(find);
			free(s);
	}
	free(csect->datas);

	for(find = list_begin(csect->modtab);
		find != list_end(csect->modtab);
		/* Do Nothing */){
			MODIFY *s = list_entry(find, MODIFY, elem);
			find = list_next(find);
			if(s->more)
				free(s->more);
			free(s);
	}
	free(csect->modtab);

	for(find = list_begin(csect->littab);
		find != list_end(csect->littab);
		/* Do Nothing */){
			LITERAL *s = list_entry(find, LITERAL, elem);
			find = list_next(find);
			// TODO REMOVE USER!
			free(s);
	}
	free(csect->littab);

	for(find = list_begin(csect->blocks);
		find != list_end(csect->blocks);
		/* Do Nothing */){
			BLOCK *s = list_entry(find, BLOCK, elem);
			find = list_next(find);
			// TODO REMOVE USER!
			free(s);
	}
	free(csect->blocks);

	free(csect->filename);

	free(csect);  // Dealloc Document.
}
