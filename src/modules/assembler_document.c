DOCUMENT *document_alloc(){
	DOCUMENT *new = (DOCUMENT *)calloc(1, sizeof(DOCUMENT));
	new->csects = (List *)calloc(1, sizeof(List));
	list_init(new->csects);
	new->shared = (List *)calloc(1, sizeof(List));
	list_init(new->shared);
	new->filename = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	return new;
}

void document_dealloc(DOCUMENT *doc){
	Elem *find;
	for(find = list_begin(doc->csects);
		find != list_end(doc->csects);
		/* Do Nothing! */){
			CSECT *csect = list_entry(find, CSECT, elem);
			find = list_next(find);
			if(0)
				csect_dealloc(csect);  // TODO!
	}
	free(doc->csects);

	// TODO shared.

	free(doc->filename);

	free(doc);
}

char *document_csect_detect(NODE *now){  // TODO POOR NAMING!!
	for(now->cur_token = 0; now->cur_token < now->token_cnt; now->cur_token++){
		if(strcasecmp(CUR(now), "CSECT") == 0){
			return CUR2(now, -1);
		}
	}
	return NULL;
}

SHARED *shared_search(List *shared, char *query){
	Elem *find;
	for(find = list_begin(shared);
		find != list_end(shared);
		/* Do Nothing! */){
			SHARED *shr = list_entry(find, SHARED, elem);
			find = list_next(find);
			if(strcasecmp(shr->symbol, query) == 0){
				return shr;
			}
	}
	return NULL;
}
