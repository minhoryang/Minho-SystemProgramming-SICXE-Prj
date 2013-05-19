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

LITERAL *literal_detect(CSECT *csect){
	NODE *now = csect->cur_block->cur_node;
	bool is_found = false;
	for(now->cur_token = 0; now->cur_token < now->token_cnt; now->cur_token++)
		if(strcasecmp(CUR(now), "=") == 0){
			is_found = true;
			break;
		}
	if(is_found)
		return literal_add(csect->littab, CUR2(now, 1));
	return NULL;
}

void literal_flush(CSECT *csect){
	NODE *new;
	Elem *find;
	for(find = list_begin(csect->littab);
		find != list_end(csect->littab);
		find = list_next(find)){
			LITERAL *this = list_entry(find, LITERAL, elem);
			if(!this->done){
				this->done = true;
				new = node_alloc();
				new->_PARENT = csect->cur_block;
				this->where = new;
				sprintf(new->token_orig, "\t=X'%s'", this->name);
				new->STORED_DATA = strdup(this->name);  // TODO!
				new->_size = strlen(this->name)/2;
				/*{
					csect->cur_block->cur_node->LOCATION_CNT = csect->cur_block->prev_locctr;
					csect->cur_block->prev_locctr = csect->cur_block->cur_node->LOCATION_CNT + csect->cur_block->cur_node->_size;
				}*/
				list_insert((csect->cur_block->cur_node->elem.next), &(new->elem));
				csect->cur_block->cur_node = new;
			}
	}
}

LITERAL *literal_dealloc(CSECT *csect){
	return NULL;
}
