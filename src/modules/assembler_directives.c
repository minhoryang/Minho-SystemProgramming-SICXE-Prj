List *assembler_directives_load(){
	List *target = (List *)calloc(1, sizeof(List));
	char wanted[6][7] = {"START", "END", "BYTE", "WORD", "RESB", "RESW"};
	ASMDirFunc wanted_func[6] = {
		assembler_directives_START,
		assembler_directives_END,
		assembler_directives_BYTE,
		assembler_directives_WORD,
		assembler_directives_RESB,
		assembler_directives_RESW};
	size_t i, wanted_cnt = 6;
	ASMDir *new;

	list_init(target);

	for(i = 0; i < wanted_cnt; i++){
		new = (ASMDir *)calloc(1, sizeof(ASMDir));
		new->apply = wanted_func[i];
		strcpy(new->name, wanted[i]);
		list_push_back(target, &(new->elem));
	}
	return target;
}

void assembler_directives_unload(List *target){
	Elem *find;
	for(find = list_begin(target);
		find != list_end(target);
		/* Do Nothing */){
			ASMDir *now = list_entry(find, ASMDir, elem);
			find = list_next(find);
			free(now);	
	}
	free(target);
}

ASMDir *assembler_directives_search(List *target, char *query){
	Elem *find;
	for(find = list_begin(target);
		find != list_end(target);
		find = list_next(find)){
			ASMDir *now = list_entry(find, ASMDir, elem);
			if(strcasecmp(now->name, query) == 0)
				return now;
	}
	return NULL;
}

void assembler_directives_START(DOCUMENT *doc, void *aux){
	doc->base = (doc->prev_locctr = hex2int((char *)aux));
	if(0)
		printf("Set Base @ %lu\t", doc->prev_locctr);
	doc->progname = strdup(doc->cur_node->token_pass[0]);
	doc->cur_node->RESERVED = true;
}

void assembler_directives_END(DOCUMENT *doc, void *aux){
	doc->cur_node->RESERVED = true;
	// TODO!!!!

	SYMBOL *wanted;
	if((wanted = symbol_search(doc->symtab, (char *)aux)) != NULL){
		doc->end_addr = wanted->link->LOCATION_CNT;
	}
}

void assembler_directives_BYTE(DOCUMENT *doc, void *aux){
	doc->cur_node->_size = strlen((doc->cur_node->STORED_DATA = strdup((char *)aux))) / 2;
	if(0)
		printf("%lu:%s\t", doc->cur_node->_size, doc->cur_node->STORED_DATA);
}

void assembler_directives_WORD(DOCUMENT *doc, void *aux){
	doc->cur_node->_size = 3;  // TODO FIXED NUMBERS!
	sprintf(
		(doc->cur_node->STORED_DATA = (char *)calloc(7, sizeof(char))),
		"%06X",
		atoi((char *)aux)
	);
	if(0)
		printf("%lu:%s\t", doc->cur_node->_size, doc->cur_node->STORED_DATA);
	
}

void assembler_directives_RESB(DOCUMENT *doc, void *aux){
	doc->cur_node->RESERVED = true;
	sscanf((char *)aux, "%lu", &(doc->cur_node->_size));
	if(0)
		printf("%lu:%s\t", doc->cur_node->_size, doc->cur_node->STORED_DATA);
}

void assembler_directives_RESW(DOCUMENT *doc, void *aux){
	doc->cur_node->RESERVED = true;
	doc->cur_node->_size = hex2int((char *)aux) * 3; 
	if(0)
		printf("%lu:%s\t", doc->cur_node->_size, doc->cur_node->STORED_DATA);
}
