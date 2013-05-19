	List *assembler_directives_load(){
	List *target = (List *)calloc(1, sizeof(List));
#define CNT 9
	char wanted[CNT][7] = {"START", "END", "BYTE", "WORD", "RESB", "RESW", "BASE", "EQU", "LTORG"};
	ASMDirFunc wanted_func[CNT] = {
		assembler_directives_START,
		assembler_directives_END,
		assembler_directives_BYTE,
		assembler_directives_WORD,
		assembler_directives_RESB,
		assembler_directives_RESW,
		assembler_directives_BASE,
		assembler_directives_EQU,
		assembler_directives_LTORG};
	size_t i;
	ASMDir *new;

	list_init(target);

	for(i = 0; i < CNT; i++){
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

void assembler_directives_START(DOCUMENT *doc){
	doc->start_addr = (doc->prev_locctr = hex2int(CUR2(doc->cur_node, 1)));
	if(DEBUG_PRINT)
		printf("Set Base @ %lu\t", doc->prev_locctr);
	doc->progname = strdup(doc->cur_node->token_pass[0]);
}

void assembler_directives_END(DOCUMENT *doc){
	SYMBOL *wanted;
	if((wanted = symbol_search(doc->symtab, CUR2(doc->cur_node, 1))) != NULL){
		doc->end_addr = wanted->link->LOCATION_CNT;
	}
	literal_flush(doc);
}

void assembler_directives_BYTE(DOCUMENT *doc){
	doc->cur_node->_size = strlen((doc->cur_node->STORED_DATA = strdup(CUR2(doc->cur_node, 1)))) / 2;
	if(DEBUG_PRINT)
		printf("%lu:%s\t", doc->cur_node->_size, doc->cur_node->STORED_DATA);
}

void assembler_directives_WORD(DOCUMENT *doc){
	doc->cur_node->_size = 3;
	sprintf(
		(doc->cur_node->STORED_DATA = (char *)calloc(7, sizeof(char))),
		"%06X",
		atoi(CUR2(doc->cur_node, 1))
	);
	if(DEBUG_PRINT)
		printf("%lu:%s\t", doc->cur_node->_size, doc->cur_node->STORED_DATA);
	
}

void assembler_directives_RESB(DOCUMENT *doc){
	doc->cur_node->FLAGS.RESERVED_SO_JMP_OBJ = true;
	sscanf(CUR2(doc->cur_node, 1), "%lu", &(doc->cur_node->_size));
	if(DEBUG_PRINT)
		printf("%lu:%s\t", doc->cur_node->_size, doc->cur_node->STORED_DATA);
}

void assembler_directives_RESW(DOCUMENT *doc){
	doc->cur_node->FLAGS.RESERVED_SO_JMP_OBJ = true;
	doc->cur_node->_size = hex2int(CUR2(doc->cur_node, 1)) * 3;
	if(DEBUG_PRINT)
		printf("%lu:%s\t", doc->cur_node->_size, doc->cur_node->STORED_DATA);
}

void assembler_directives_BASE(DOCUMENT *doc){
	assembler_directives_BASE_TO_BE(doc, true);
}

void assembler_directives_BASE_TO_BE(DOCUMENT *doc, bool first){
	DATA *got;
	// TODO : Refactoring Needed!
	if(first){
		got = assembler_get_value_from_symbol_or_not(doc, CUR2(doc->cur_node, 1));
		if(got->where == FAILED){
			doc->is_base = UseSymbol;
			doc->to_base = doc->cur_node;
		}else{
			doc->is_base = Set;
			doc->base = got->wanted;
		}
	}else{
		if(doc->is_base == UseSymbol){
			got = assembler_get_value_from_symbol_or_not(doc, CUR2(doc->to_base, 1));
			if(got->where == FAILED){
				printf("ERR!!!!!!!!!!!!!!!");
			}else{
				doc->is_base = Set;
				doc->base = got->wanted;
			}
		}
	}
	if(DEBUG_PRINT)
		printf("[[[%lu]]]\t", doc->base);
}

void assembler_directives_EQU(DOCUMENT *doc){
	printf("\n<%s>\n", doc->cur_node->token_orig);
	doc->cur_node->Symbol->is_equ = true;
	if(strcasecmp(CUR2(doc->cur_node, 1), "*") == 0){
		doc->cur_node->Symbol->equ = doc->prev_locctr;
	}else{
		// LOOP for Handling +-....
		doc->cur_node->Symbol->equ = plus_minus_shit_parade(doc);
	}
}

void assembler_directives_LTORG(DOCUMENT *doc){
	literal_flush(doc);
}

int plus_minus_shit_parade(DOCUMENT *doc){
	NODE *now = doc->cur_node;
	bool is_plus = true;
	int a = 0, b = 0;
	for(++now->cur_token; now->cur_token < now->token_cnt; now->cur_token++){
		printf("<%s>\t", CUR(now));
		if(strcasecmp(CUR(now), "+") == 0){
			if(is_plus)
				a += b;
			else
				a -= b;
			is_plus = true;
		}else if(strcasecmp(CUR(now), "-") == 0){
			if(is_plus)
				a += b;
			else
				a -= b;
			is_plus = false;
		}else{
			DATA *got = assembler_get_value_from_symbol_or_not(doc, CUR(now));
			switch(got->where){
				case FAILED:
					printf("ERRRRRRRR!\n");  // TODO!
					break;
				case Integer:
				case Symbol:
				case Literal:
					b = got->wanted;
					break;
			}
			printf("b%d\t", b);
		}
	}
	if(is_plus)
		a += b;
	else
		a -= b;
	return a;
}
