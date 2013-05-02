#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "modules/assembler.h"
#include "modules/tokenizer.h"

#ifdef assembler_test
	#include "modules/optab.c"
	#include "modules/tokenizer.c"
	int main(){
		char *filename = "2-1.asm", 
			 *filename1 = strdup(filename),
			 *filename2 = strdup(filename);
		strcat(filename1, ".lst");
		strcat(filename2, ".obj");
		
		DOCUMENT *doc;

		Hash *OP = OP_Alloc();  // OPCode Load.
		OPMN_Load(OP, NULL);

		List *directives = assembler_directives_load();
		
		assembler_readline(filename, (doc = document_alloc()));
		assembler_pass1(doc, OP, directives);
		assembler_pass2(doc, filename1);
		assembler_pass3(doc, filename2);

		hash_destroy(OP, both_hash_destructor);  // OPCode Unload.
		assembler_directives_unload(directives);
		document_dealloc(doc);
		return 0;
	}
#endif

bool assembler_readline(char *filename, DOCUMENT *doc){
	FILE *fin = fopen(filename, "r");
	size_t now = 0, jmp = 5;

	if(fin == NULL) return true;

	while(
		fgets(
			(doc->cur_node = node_alloc())->token_orig,
			Tokenizer_Max_Length,
			fin) != NULL){
				// NoEnter.
				char *new = Tokenizer_NoEnter((doc->cur_node)->token_orig);
				free((doc->cur_node)->token_orig);
				(doc->cur_node)->token_orig = new;
				// Readline.
				list_push_back(doc->nodes, &((doc->cur_node)->elem));
				(doc->cur_node)->token_cnt = Tokenizer(
									(doc->cur_node)->token_orig,
									(doc->cur_node)->token_pass,
									true);
				now = ((doc->cur_node)->LINE_NUM = now + jmp);
	}
	doc->cur_node = NULL;
	strncpy(doc->filename, filename, strlen(filename) - 4);
	fclose(fin);
	return false;
}

bool assembler_pass1(DOCUMENT *doc, Hash *opcode, List *asmdirs){
	Elem *find;
	bool emergency = false;

	for(find = list_begin(doc->nodes);
		find != list_end(doc->nodes);
		find = list_next(find)){
			size_t i = 0;
			bool got_symbol = false, got_asmdir = false, got_op = false;
			NODE *now = (doc->cur_node = list_entry(find, NODE, elem));

			for(i = 0; i < now->token_cnt; i++){
				OPMNNode *found_op;
				ASMDir *found_asmdir;
				SYMBOL *new_symbol;
				if(!got_symbol){  // 1. NO SYMBOL
					if(!got_asmdir && !got_op){  // 1-A. NO SYMBOL & NO CMD
						// need to find 'CMD' or 'SYMBOL'.
						if((found_op = MN_Search(opcode, now->token_pass[i])) != NULL){ // 1-A-a. NO SYMBOL, GOT OPCODE.
							if(0)
								printf("OPCODE! %s\t", now->token_pass[i]);
							got_op = true;
							now->OPCODE = found_op;
							now->_size = 3;  // TODO!
						}else if((found_asmdir = assembler_directives_search(asmdirs, now->token_pass[i])) != NULL){  // 1-A-b. NO SYMBOL, GOT ASMDIR.
							if(0)
								printf("ASMDIR! %s\t", now->token_pass[i]);
							got_asmdir = true;
							found_asmdir->apply(doc, (void *)now->token_pass[i+1]);
						}else if(strcmp(".", now->token_pass[i]) == 0){  // 1-A-c. NO SYMBOL, USELESS COMMENT LINE.
							now->COMMENTED = true;
							break;
						}else{  // 1-A-d. << SHOULD BE SYMBOL >>
							if(0)
								printf("SYMBOL! %s\t", now->token_pass[i]);
							got_symbol = true;
							if((new_symbol = symbol_add(doc->symtab, now->token_pass[i], now, true)) != NULL){
								now->Symbol = new_symbol;
								new_symbol = NULL;
							}else{
								printf("ERROR!!!! LINE %lu : RIGHT AFTER SYMBOL's POSITION, NEED TO FOLLOW MNEMONIC/ASMDIRs.\n", now->LINE_NUM);
								emergency = true;
							}
						}
					}else{
						// rest of it would be 'DISP'.
						// --------------------------------
						if(got_op){  // 1-B. NO SYMBOL, OPCODE WORKS!
							if(strcmp("X", now->token_pass[i]) == 0){  // TODO 0->SAME
								(now->FLAGS)._X_ = true;
								if(0)
									printf("_X_\t");
							}else if(strcmp(".", now->token_pass[i]) == 0){
								;
							}else if((new_symbol = symbol_add(doc->symtab, now->token_pass[i], NULL, false)) != NULL){
								now->DISP = new_symbol;
								new_symbol = NULL;
							}else{
								printf("2ERROR!!!! LINE %lu\n", now->LINE_NUM);
								emergency = true;
							}
						}else if(got_asmdir){  // 1-C. NO SYMBOL, ASMDIR WORKS!
							// XXX : IGNORE for 'END' and 'EQU', ...
						}else{  // 1-D. NO SYMBOL, NO CMD!
							printf("3ERROR!!!! LINE %lu\n", now->LINE_NUM);
							emergency = true;
						}
						// --------------------------------
					}
				}else{
					if(!got_asmdir && !got_op){
						// have to be 'CMD' here!
						if((found_op = MN_Search(opcode, now->token_pass[i])) != NULL){
							got_op = true;
							if(0)
								printf("OPCODE! %s\t", now->token_pass[i]);
							now->OPCODE = found_op;
							now->_size = 3;  // TODO!
						}else if((found_asmdir = assembler_directives_search(asmdirs, now->token_pass[i])) != NULL){
							got_asmdir = true;
							if(0)
								printf("ASMDIR! %s\t", now->token_pass[i]);
							found_asmdir->apply(doc, (void *)now->token_pass[i+1]);
						}else{
							printf("4ERROR!!!! LINE %lu\n", now->LINE_NUM);
							emergency = true;
						}
					}else{
						// rest of it would be 'DISP'.
						// --------------------------------
						if(got_op){
							if(strcmp("X", now->token_pass[i]) == 0){  // TODO 0->SAME
								(now->FLAGS)._X_ = true;
								if(0)
									printf("_X_\t");
							}else if(strcmp(".", now->token_pass[i]) == 0){
								;
							}else if((new_symbol = symbol_add(doc->symtab, now->token_pass[i], NULL, false)) != NULL){
								now->DISP = new_symbol;
								new_symbol = NULL;
							}else{
								printf("5ERROR!!!! LINE %lu\n", now->LINE_NUM);
								emergency = true;
							}
						}else if(got_asmdir){
							// XXX : IGNORE for 'END' and 'EQU', ...
						}else{
							printf("6ERROR!!!! LINE %lu\n", now->LINE_NUM);
							emergency = true;
						}
						// --------------------------------
					}
				}
			}
			now->LOCATION_CNT = doc->prev_locctr;
			doc->prev_locctr = now->LOCATION_CNT + now->_size;
			if(0)
				printf(" %04X\n", (unsigned int)now->LOCATION_CNT);
			if(emergency)
				break;
	}
	doc->cur_node = NULL;
	return emergency;
}

bool assembler_pass2(DOCUMENT *doc, char *filename){
	Elem *find;
	FILE *fout = fopen(filename, "w");

	for(find = list_begin(doc->nodes);
		find != list_end(doc->nodes);
		find = list_next(find)){
			NODE *now = (doc->cur_node = list_entry(find, NODE, elem));
			fprintf(fout, "%5lu\t", now->LINE_NUM); 
			if(!now->COMMENTED){
				fprintf(fout, "%04X", (unsigned int)(now->LOCATION_CNT)); 
			}
			fprintf(fout, "\t%s\t", now->token_orig); 
			if(now->STORED_DATA){
				sprintf(now->OBJECTCODE, "%s", now->STORED_DATA);
			}else{
				if(now->OPCODE){
					if(now->DISP){
						if(!now->DISP->link){
							printf("7ERROR!!!! LINE %lu DISP %s\n", now->LINE_NUM, now->DISP->symbol);
							return true;
						}
						if((now->FLAGS)._X_){
							sprintf(now->OBJECTCODE,
									"%02X%04X",
									now->OPCODE->opcode,
									(unsigned int)now->DISP->link->LOCATION_CNT + 0x8000);
						}else{
							sprintf(now->OBJECTCODE,
									"%02X%04X",
									now->OPCODE->opcode,
									(unsigned int)now->DISP->link->LOCATION_CNT);
						}
					}else{
						sprintf(now->OBJECTCODE, "%02X0000", now->OPCODE->opcode);
					}
				}
			}
			fprintf(fout, "%s", now->OBJECTCODE);
			fprintf(fout, "\n");
	}
	doc->cur_node = NULL;
	fclose(fout);
	return false;
}

void assembler_pass3(DOCUMENT *doc, char *filename){
	Elem *find;
	FILE *fout = fopen(filename, "w");
	size_t cnt = 0, max_cnt = 60;
	Elem *line_from = NULL, *line_end = NULL;
	bool is_storing = false;

	fprintf(fout, "H%-6s%06X%06X\n", doc->progname, (unsigned int)doc->base, (unsigned int)(doc->prev_locctr - doc->base));

	for(find = list_begin(doc->nodes);
		find != list_end(doc->nodes);
		find = list_next(find)){
			NODE *now = (doc->cur_node = list_entry(find, NODE, elem));
			if(now->RESERVED){
				// 정산!
				if(is_storing){
					is_storing = false;
					// 출력함수 line_from ~ line_end;
					assembler_pass3_print(fout, line_from, line_end, cnt);
				}
			}else{
				if(!is_storing){
					is_storing = true;
					line_from = find;
					line_end = find;
					cnt = strlen(now->OBJECTCODE);
				}else{
					if(cnt + strlen(now->OBJECTCODE) > max_cnt){
						is_storing = false;
						// 출력함수 line_from ~ line_end;
						assembler_pass3_print(fout, line_from, line_end, cnt);
						is_storing = true;
						line_from = find;
						line_end = find;
						cnt = strlen(now->OBJECTCODE);
					}else{
						line_end = find;
						cnt += strlen(now->OBJECTCODE);
					}
				}
			}
	}
	fprintf(fout, "E%06X\n", (unsigned int)doc->end_addr);
	doc->cur_node = NULL;

	//fclose(fout);
}

void assembler_pass3_print(FILE *fp, Elem *start, Elem *end, size_t cnt){
	Elem *find;
	NODE *one = list_entry(start, NODE, elem);
	fprintf(fp, "T%06X%02X", (unsigned int)one->LOCATION_CNT, (unsigned int)cnt/2);

	for(find = start;
		find != list_next(end);
		find = list_next(find)){
			NODE *now = list_entry(find, NODE, elem);
			fprintf(fp, "%s", now->OBJECTCODE);
	}
	fprintf(fp, "\n");
}

DOCUMENT *document_alloc(){
	DOCUMENT *new = (DOCUMENT *)calloc(1, sizeof(DOCUMENT));
	new->nodes = (List *)calloc(1, sizeof(List));
	list_init(new->nodes);
	new->symtab = (List *)calloc(1, sizeof(List));
	list_init(new->symtab);
	new->filename = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	return new;
}

void document_dealloc(DOCUMENT *doc){
	struct list_elem *find;

	for(find = list_begin(doc->nodes);
		find != list_end(doc->nodes);
		/* Do Nothing */){
			doc->cur_node = list_entry(find, NODE, elem);
			find = list_next(find);  // go farther before deleted.
			node_dealloc(doc->cur_node);  // Dealloc NODE.
	}
	doc->cur_node = NULL;
	free(doc->nodes);

	for(find = list_begin(doc->symtab);
		find != list_end(doc->symtab);
		/* Do Nothing */){
			SYMBOL *s = list_entry(find, SYMBOL, elem);
			find = list_next(find);
			if(s->symbol)
				free(s->symbol);
			free(s);
	}
	free(doc->symtab);

	free(doc->filename);

	free(doc);  // Dealloc Document.
}

NODE *node_alloc(){
	NODE *node = (NODE *)calloc(1, sizeof(NODE));
	node->token_orig = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	node->token_pass = AllocToken();
	node->OBJECTCODE = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	return node;
}

void node_dealloc(NODE *node){
	//size_t i;
	//printf("%lu %s\n", node->LINE_NUM, node->token_orig);
	//for(i = 0; i < node->token_cnt; i++){
	//	printf("%lu:%s\n", i, node->token_pass[i]);
	//}
	DeAllocToken(node->token_pass);
	free(node->OBJECTCODE);
	free(node->token_orig);
	free(node);
}

#include "modules/assembler_directives.c"
#include "modules/assembler_symbol.c"
