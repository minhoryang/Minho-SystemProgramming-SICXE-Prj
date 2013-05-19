BLOCK *block_alloc(size_t ID, char *name){
	BLOCK *new = (BLOCK *)calloc(1, sizeof(BLOCK));
	new->NAME = strdup(name);
	new->ID = ID;
	new->nodes = (List *)calloc(1, sizeof(List));
	list_init(new->nodes);
	return new;
}

char *block_detect(NODE *node){
	for(node->cur_token = 0; node->cur_token < node->token_cnt; node->cur_token++){
		if(strcasecmp(CUR(node), "USE") == 0){
			if(strcasecmp(CUR2(node, 1), "") == 0){
				return "(default)";
			}else{
				return CUR2(node, 1);
			}
		}
	}
	return NULL;
}

BLOCK *block_search(List *blocks, char *query){
	Elem *find;
	for(find = list_begin(blocks);
		find != list_end(blocks);
		find = list_next(find)){
			BLOCK *found = list_entry(find, BLOCK, elem);
			if(strcasecmp(found->NAME, query) == 0)
				return found;
	}
	return NULL;
}

void block_dealloc(BLOCK *block){
	;
}
