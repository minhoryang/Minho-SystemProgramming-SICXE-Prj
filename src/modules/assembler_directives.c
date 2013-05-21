List *assembler_directives_load(){
	List *target = (List *)calloc(1, sizeof(List));
#define CNT 14
	char wanted[CNT][7] = {"START", "END", "BYTE", "WORD", "RESB", "RESW", "BASE", "EQU", "LTORG", "USE", "ORG", "CSECT", "EXTDEF", "EXTREF"};
	ASMDirFunc wanted_func[CNT] = {
		assembler_directives_START,
		assembler_directives_END,
		assembler_directives_BYTE,
		assembler_directives_WORD,
		assembler_directives_RESB,
		assembler_directives_RESW,
		assembler_directives_BASE,
		assembler_directives_EQU,
		assembler_directives_LTORG,
		assembler_directives_USE,
		assembler_directives_ORG,
		assembler_directives_CSECT,
		assembler_directives_EXTDEF,
		assembler_directives_EXTREF};
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
	CSECT *csect = doc->cur_csect;
	csect->start_addr = (csect->prev_base = hex2int(CUR2(csect->cur_block->cur_node, 1)));
	if(DEBUG_PRINT)
		printf("Set Base @ %lu\t", csect->cur_block->prev_locctr);
	csect->progname = strdup(csect->cur_block->cur_node->token_pass[0]);

	SHARED *new = (SHARED *)calloc(1, sizeof(SHARED));
	new->symbol = strdup(csect->progname);
	list_push_back(doc->shared, &(new->elem));
}

void assembler_directives_END(DOCUMENT *doc){
	SYMBOL *wanted;
	CSECT *csect = doc->cur_csect;
	if((wanted = symbol_search(csect->symtab, CUR2(csect->cur_block->cur_node, 1))) != NULL){
		csect->end_addr = wanted->link->LOCATION_CNT;
	}
	literal_flush(csect);
}

void assembler_directives_BYTE(DOCUMENT *doc){
	CSECT *csect = doc->cur_csect;
	csect->cur_block->cur_node->_size = strlen((csect->cur_block->cur_node->STORED_DATA = strdup(CUR2(csect->cur_block->cur_node, 1)))) / 2;
	if(DEBUG_PRINT)
		printf("%lu:%s\t", csect->cur_block->cur_node->_size, csect->cur_block->cur_node->STORED_DATA);
}

void assembler_directives_WORD(DOCUMENT *doc){
	CSECT *csect = doc->cur_csect;
	csect->cur_block->cur_node->_size = 3;
	sprintf(
		(csect->cur_block->cur_node->STORED_DATA = (char *)calloc(7, sizeof(char))),
		"%06X",
		plus_minus_shit_parade(doc)
	);
	if(DEBUG_PRINT)
		printf("%lu:%s\t", csect->cur_block->cur_node->_size, csect->cur_block->cur_node->STORED_DATA);
	
}

void assembler_directives_RESB(DOCUMENT *doc){
	CSECT *csect = doc->cur_csect;
	csect->cur_block->cur_node->FLAGS.RESERVED_SO_JMP_OBJ = true;
	sscanf(CUR2(csect->cur_block->cur_node, 1), "%lu", &(csect->cur_block->cur_node->_size));
	if(DEBUG_PRINT)
		printf("%lu:%s\t", csect->cur_block->cur_node->_size, csect->cur_block->cur_node->STORED_DATA);
}

void assembler_directives_RESW(DOCUMENT *doc){
	CSECT *csect = doc->cur_csect;
	csect->cur_block->cur_node->FLAGS.RESERVED_SO_JMP_OBJ = true;
	csect->cur_block->cur_node->_size = hex2int(CUR2(csect->cur_block->cur_node, 1)) * 3;
	if(DEBUG_PRINT)
		printf("%lu:%s\t", csect->cur_block->cur_node->_size, csect->cur_block->cur_node->STORED_DATA);
}

void assembler_directives_BASE(DOCUMENT *doc){
	assembler_directives_BASE_TO_BE(doc, true);
}

bool assembler_directives_BASE_TO_BE(DOCUMENT *doc, bool first){
	CSECT *csect = doc->cur_csect;
	DATA *got;
	// TODO : Refactoring Needed!
	if(first){
		got = assembler_get_value_from_symbol_or_not(csect, CUR2(csect->cur_block->cur_node, 1));
		if(got->where == FAILED){
			csect->is_base = UseSymbol;
			csect->to_base = csect->cur_block->cur_node;
		}else{
			csect->is_base = Set;
			csect->base = got->wanted;
		}
	}else{
		if(csect->is_base == UseSymbol){
			got = assembler_get_value_from_symbol_or_not(csect, CUR2(csect->to_base, 1));
			if(got->where == FAILED){
				printf("ERR! @ LINE NUM %lu\n", csect->to_base->LINE_NUM);
				return false;
			}else{
				csect->is_base = Set;
				csect->base = got->wanted;
			}
		}
	}
	if(DEBUG_PRINT)
		printf("[[[%lu]]]\t", csect->base);
	return true;
}

void assembler_directives_EQU(DOCUMENT *doc){
	CSECT *csect = doc->cur_csect;
	csect->cur_block->cur_node->Symbol->is_equ = true;
	if(strcasecmp(CUR2(csect->cur_block->cur_node, 1), "*") == 0){
		csect->cur_block->cur_node->Symbol->equ = csect->cur_block->prev_locctr;
	}else{
		// LOOP for Handling +-....
		csect->cur_block->cur_node->Symbol->equ = plus_minus_shit_parade(doc);
	}
}

void assembler_directives_LTORG(DOCUMENT *doc){
	CSECT *csect = doc->cur_csect;
	literal_flush(csect);
}

int plus_minus_shit_parade(DOCUMENT *doc){
	CSECT *csect = doc->cur_csect;
	NODE *now = csect->cur_block->cur_node;
	bool is_plus = true;
	int a = 0, b = 0;
	for(++now->cur_token; now->cur_token < now->token_cnt; now->cur_token++){
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
			DATA *got = assembler_get_value_from_symbol_or_not(csect, CUR(now));
			switch(got->where){
				case FAILED:
					printf("ERR! CALC FAILED!! LINE NUM %lu\n", now->LINE_NUM);
					break;
				case Integer:
				case Symbol:
				case Literal:
				case Shared:  // TODO
					b = got->wanted;
					break;
					break;
			}
		}
	}
	if(is_plus)
		a += b;
	else
		a -= b;
	return a;
}

void assembler_directives_USE(DOCUMENT *doc){
	// DO NOTHING!
}

void assembler_directives_ORG(DOCUMENT *doc){
	CSECT *csect = doc->cur_csect;
	csect->cur_block->prev_locctr = plus_minus_shit_parade(doc);
	// DO NOTHING!
}

void assembler_directives_CSECT(DOCUMENT *doc){
	CSECT *csect = doc->cur_csect;
	csect->progname = strdup(csect->cur_block->cur_node->token_pass[0]);
	// DO NOTHING!
}

void assembler_directives_EXTDEF(DOCUMENT *doc){
	NODE *now = doc->cur_csect->cur_block->cur_node;
	for(++now->cur_token; now->cur_token < now->token_cnt; now->cur_token++){
		{
			DEFINE *new = (DEFINE *)calloc(1, sizeof(DEFINE));
			new->symbol = strdup(CUR(now));
			list_push_back(doc->cur_csect->define, &(new->elem));
		}
		{
			SHARED *new = (SHARED *)calloc(1, sizeof(SHARED));
			new->symbol = strdup(CUR(now));
			list_push_back(doc->shared, &(new->elem));
		}
	}
	// DO NOTHING!
}

void assembler_directives_EXTREF(DOCUMENT *doc){
	NODE *now = doc->cur_csect->cur_block->cur_node;
	for(++now->cur_token; now->cur_token < now->token_cnt; now->cur_token++){
		DEFINE *new = (DEFINE *)calloc(1, sizeof(DEFINE));
		new->symbol = strdup(CUR(now));
		list_push_back(doc->cur_csect->reference, &(new->elem));
	}
	// DO NOTHING!
}

