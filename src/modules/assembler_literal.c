LITERAL *literal_add(List *littab, char *query){
	LITERAL *result;
	if((result = literal_search(littab, query)) == NULL){
		result = (LITERAL *)calloc(1, sizeof(LITERAL));
		result->name = strdup(query);
		result->done = false;
		list_push_back(littab, &(result->elem));
	}
	return result;
}

LITERAL *literal_search(List *littab, char *query){
	Elem *find;
	for(find = list_begin(littab);
		find != list_end(littab);
		find = list_next(find)){
			LITERAL *this = list_entry(find, LITERAL, elem);
			if(strcasecmp(this->name, query) == 0){
				return this;
			}
	}
	return NULL;
}

LITERAL *literal_detect(DOCUMENT *doc){
	NODE *now = doc->cur_block->cur_node;
	bool is_found = false;
	for(now->cur_token = 0; now->cur_token < now->token_cnt; now->cur_token++)
		if(strcasecmp(CUR(now), "=") == 0){
			is_found = true;
			break;
		}
	if(is_found)
		return literal_add(doc->littab, CUR2(now, 1));
	return NULL;
}

void literal_flush(DOCUMENT *doc){
	NODE *new;
	Elem *find;
	for(find = list_begin(doc->littab);
		find != list_end(doc->littab);
		find = list_next(find)){
			LITERAL *this = list_entry(find, LITERAL, elem);
			if(!this->done){
				this->done = true;
				new = node_alloc();
				new->_PARENT = doc->cur_block;
				this->where = new;
				sprintf(new->token_orig, "\t=X'%s'", this->name);
				new->STORED_DATA = strdup(this->name);  // TODO!
				new->_size = strlen(this->name)/2;
				/*{
					doc->cur_block->cur_node->LOCATION_CNT = doc->cur_block->prev_locctr;
					doc->cur_block->prev_locctr = doc->cur_block->cur_node->LOCATION_CNT + doc->cur_block->cur_node->_size;
				}*/
				list_insert((doc->cur_block->cur_node->elem.next), &(new->elem));
				doc->cur_block->cur_node = new;
			}
	}
}

LITERAL *literal_dealloc(DOCUMENT *doc){
	return NULL;
}
