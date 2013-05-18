#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "modules/assembler.h"
#include "modules/tokenizer.h"

#ifdef assembler_test
	#include "modules/optab.c"
	#include "modules/tokenizer.c"
	int main(){
		bool select_test = false;
		if(select_test){
			char *filename = "2-1.asm",   // TODO FILENAME
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
			assembler_make_lst(doc, filename1);
			assembler_make_obj(doc, filename2);

			hash_destroy(OP, both_hash_destructor);  // OPCode Unload.
			assembler_directives_unload(directives);
			document_dealloc(doc);
		}else{
			Hash *OP = OP_Alloc();  // OPCode Load.
			Hash *MN = MN_Alloc();
			OPMN_Load(OP, MN);
			// disassembler("objobj", MN);
			hash_destroy(OP, both_hash_destructor);  // OPCode Unload.
			hash_destroy(MN, NULL);
		}
		return 0;
	}
#endif

bool assembler_readline(char *filename, DOCUMENT *doc){
	// XXX : Read Line -> Assign Node -> set$lineNum$ -> Add Node @ DOCUMENT.
	FILE *fin = fopen(filename, "r");
	size_t lineNum = 0, jmp = 5;
	enum ThreeStates {reset=-1, no, yes} lineNumIn = reset;

	if(fin == NULL) return true;

	while(
		fgets(  // Allocate and Get 1 line.
			(doc->cur_node = node_alloc())->token_orig,
			Tokenizer_Max_Length,
			fin) != NULL){
				{  // Pass tokenier for 'Enter' and Swap these.
					char *new = Tokenizer_NoEnter((doc->cur_node)->token_orig);
					free((doc->cur_node)->token_orig);
					(doc->cur_node)->token_orig = new;
				}
				{  // Add node to Document.
					list_push_back(doc->nodes, &((doc->cur_node)->elem));
					(doc->cur_node)->token_cnt = Tokenizer(
									(doc->cur_node)->token_orig,
									(doc->cur_node)->token_pass,
									SIC);
					{  // XXX : LineNumber Generator.
						char *swap;
						if(lineNumIn == reset){  // ONE-TIME-ONLY. Try to find LineNum at inputs.
							if((doc->cur_node)->token_cnt > 0){
								if(IsNumberOnly((doc->cur_node)->token_pass[0]))
									lineNumIn = yes;
								else
									lineNumIn = no;
								if(DEBUG_PRINT)
									printf("%s\n", lineNumIn ? "yes" : "no");
							}
						}else{/* Nothing!, switch-case handle rest. */}
						switch(lineNumIn){
							case reset:
								// HANDLED ONE-TIME-ONLY.
								break;
							case yes:
								(doc->cur_node)->LINE_NUM = (unsigned int)atoi((doc->cur_node)->token_pass[0]);
								if((doc->cur_node)->token_cnt)
								{  // IGNORE LINENUMBER FOR ASSEMBLER_PASS.
									{  // Shift << 'LineNum'.
										swap = (doc->cur_node)->token_pass[0];  // 0
										for(jmp = 1; jmp < Tokenizer_Max_Length; jmp++)  // 1~81=>0~80
											(doc->cur_node)->token_pass[jmp-1] = (doc->cur_node)->token_pass[jmp];
										(doc->cur_node)->token_pass[Tokenizer_Max_Length-1] = swap;  // 81=>0.
									}
									(doc->cur_node)->token_cnt--;  // Set 'ignore' as decreasing token_cnt.
								}
								break;
							case no:
								lineNum = ((doc->cur_node)->LINE_NUM = lineNum + jmp);
								break;
						}
					}
				}
	}
	doc->cur_node = NULL;
	strncpy(doc->filename, filename, strlen(filename) - 4);  // TODO FILENAME
	fclose(fin);
	return false;
}

bool IsNumberOnly(char * input){  // TODO PULL THIS OUT!
	bool is_lineNum = true;
	char *n;
	for(n=input; *n != '\0'; n++){  // LineNum Checker.
		is_lineNum &= (bool)isdigit(*n);
		if(!is_lineNum)
			break;
	}
	return is_lineNum;
}

bool assembler_pass1(DOCUMENT *doc, Hash *opcode, List *asmdirs){
	// XXX : Pass 1.
	Elem *find;
	bool OMGflag = false;

	for(find = list_begin(doc->nodes);
		find != list_end(doc->nodes);
		find = list_next(find)){

			bool hasSymbol = false, hasAsmdir = false, hasOpcode = false;
			NODE *now = (doc->cur_node = list_entry(find, NODE, elem));

			for(now->cur_token = 0; now->cur_token < now->token_cnt; now->cur_token++){
				OPMNNode *found_opcode;
				ASMDir *found_asmdir;
				SYMBOL *new_symbol;

				if(!hasSymbol){  // 1. NO SYMBOL
					if(!hasAsmdir && !hasOpcode){  // 1-A. NO SYMBOL & NO CMD
						// need to find 'CMD' or 'SYMBOL'.
#define IF_OPCODE_OR_ASMDIR_DO \
						if((found_opcode = MN_Search(opcode, CUR(now))) != NULL){	\
							if(DEBUG_PRINT)	\
								printf("OPCODE! %s\t", CUR(now));	\
							hasOpcode = true;	\
							now->OPCODE = found_opcode;	\
							if(now->FLAGS._E_)	\
								now->_size = 4;	\
							else	\
								now->_size = now->OPCODE->size; \
							break;	\
						}else if((found_asmdir = assembler_directives_search(asmdirs, CUR(now))) != NULL){	\
							if(DEBUG_PRINT)	\
								printf("ASMDIR! %s\t", CUR(now));	\
							hasAsmdir = true;	\
							found_asmdir->apply(doc);	\
							break;	\
						}	\
						else if(strcasecmp("+", CUR(now)) == 0){	\
							now->FLAGS._E_ = true;	\
						}  // SET AWESOME MACRO for REDUCING DUPLICATED AREA!

						IF_OPCODE_OR_ASMDIR_DO  // if (above)
						else if(strcasecmp(".", CUR(now)) == 0){
							now->FLAGS.COMMENTED_SO_JMP_LST = true;
							break;
						}else{  // 1-A-d. << SHOULD BE SYMBOL >>
							if(DEBUG_PRINT)
								printf("SYMBOL! %s\t", CUR(now));
							hasSymbol = true;
							if((new_symbol = symbol_add(doc->symtab, CUR(now), now)) != NULL){
								now->Symbol = new_symbol;
								new_symbol = NULL;
							}else{
								printf("ERROR!!!! LINE %lu : RIGHT AFTER SYMBOL's POSITION, NEED TO FOLLOW MNEMONIC/ASMDIRs.\n", now->LINE_NUM);
								OMGflag = true;
								break;
							}
						}
					}else{  // NO SYMBOL, GOT CMD.
#ifdef DEPRECATED_SIC
						if(hasOpcode){  // 1-B. NO SYMBOL, OPCODE WORKS!
							if((OMGflag = assembler_pass1_got_opcode_check_disp(doc, now))){
								break;
							}
						}else if(hasAsmdir){  // 1-C. NO SYMBOL, ASMDIR WORKS!
							// XXX : IGNORE for 'END' and 'EQU', ...
						}else{  // 1-D. NO SYMBOL, NO CMD!
							printf("3ERROR!!!! LINE %lu\n", now->LINE_NUM);
							OMGflag = true;
							break;
						}
#endif
					}
				}else{
					if(!hasAsmdir && !hasOpcode){
						IF_OPCODE_OR_ASMDIR_DO  // if (above)
						else{
							printf("4ERROR!!!! LINE %lu\n", now->LINE_NUM);
							size_t i = 0;
							if(DEBUG_PRINT)
								for(i=0;i<now->token_cnt;i++)
									printf("<%s>\n",now->token_pass[i]);
							
							OMGflag = true;
							break;
						}
					}else{
#ifdef DEPRECATED_SIC
						// rest of it would be 'DISP'.
						// --------------------------------
						if(hasOpcode){
							if((OMGflag = assembler_pass1_got_opcode_check_disp(doc, now))){
								printf("5ERROR!!!! LINE %lu\n", now->LINE_NUM);
								break;
							}
						}else if(hasAsmdir){
							// XXX : IGNORE for 'END' and 'EQU', ...
						}else{
							printf("6ERROR!!!! LINE %lu\n", now->LINE_NUM);
							OMGflag = true;
							break;
						}
						// --------------------------------
#endif
					}
				}
			}
			now->LOCATION_CNT = doc->prev_locctr;
			doc->prev_locctr = now->LOCATION_CNT + now->_size;
			if(DEBUG_PRINT)
				printf(" %04X\n", (unsigned int)now->LOCATION_CNT);
			if(OMGflag)
				break;
	}
	doc->cur_node = NULL;
	return OMGflag;
}

#ifdef DEPRECATED_SIC
bool assembler_pass1_got_opcode_check_disp(DOCUMENT *doc, NODE *now){
	bool OMGflag = false;
	SYMBOL *new_symbol;

	if(strcasecmp("X", CUR(now)) == 0){
		(now->FLAGS)._X_ = true;
		if(DEBUG_PRINT)
			printf("_X_\t");
	}else if(strcasecmp(".", CUR(now)) == 0){
		// IGNORE COMMENT!
	}else if((new_symbol = symbol_add(doc->symtab, CUR(now), NULL)) != NULL){
		now->DISP = new_symbol;
		new_symbol = NULL;
	}else{
		printf("2ERROR!!!! LINE %lu\n", now->LINE_NUM);
		OMGflag = true;
	}
	return OMGflag;
}
#endif

bool assembler_pass2(DOCUMENT *doc){
	// XXX : Pass 2.
	Elem *find;
	bool OMGflag = false;

	assembler_directives_BASE_TO_BE(doc, false);  // TODO ERROR HANDLING!

	for(find = list_begin(doc->nodes);
		find != list_end(doc->nodes);
		find = list_next(find)){

			NODE *now = (doc->cur_node = list_entry(find, NODE, elem));
			if(now->OPCODE != NULL){
				switch(now->_size){
					case 1:
						sprintf(now->OBJECTCODE, "%02X", now->OPCODE->opcode);
						break;
					case 2:
						{
							int a = 0, b = 0;
							switch(now->token_cnt - (++now->cur_token)){
								case 2:
									a = assembler_get_value_from_register(CUR(now));
									b = assembler_get_value_from_register(CUR2(now, 1));
									break;
								case 1:
									a = assembler_get_value_from_register(CUR(now));
									break;
								default:
									// TODO : ERROR!!!!
									break;
							}
							sprintf(now->OBJECTCODE, "%02X%01X%01X", now->OPCODE->opcode, a, b);
						}
						break;
					case 4:
						now->FLAGS._E_ = true;
						// XXX : Exception - [MODIFICATION]
#define F4_EXC_MODIFY	if(now->FLAGS._E_){ \
							if(value->where == FromSymbol){ \
								MODIFY *md = (MODIFY *)calloc(1, sizeof(MODIFY)); \
								md->target = now; \
								md->more = NULL; \
								list_push_back(doc->modtab, &(md->elem)); \
							} \
						}
					case 3:
						{
							size_t need_to_find = assembler_pass2_set_flag(now);
							DATA *value = NULL;
							if(need_to_find != SIZE_MAX)
								value = assembler_get_value_from_symbol_or_not(doc, now->token_pass[need_to_find]);
							{
								int disp = 0;
								if(value){
									disp = value->wanted;
									if(value->where != Integer){
										int PC = (now->LOCATION_CNT + now->_size);
										if((-2048 <= disp - PC) && (disp - PC <= 2047)){
											disp -= PC;
											now->FLAGS._P_ = true;
										}else if((doc->is_base == Set) && (0 <= disp - doc->base) && (disp - doc->base <= 4095)){
											disp -= doc->base;
											now->FLAGS._B_ = true;
										}else{
											// TODO ERROR!
											assembler_pass2_debug_print(now);
										}
									}
								}
								assembler_pass2_object_print(now, disp);
							}
							F4_EXC_MODIFY // XXX : Exception - [MODIFICATION] @ FORMAT 4.
						}
						break;
					default:
						break;
				}
			}
	}
	doc->cur_node = NULL;
	return OMGflag;
}

size_t assembler_pass2_set_flag(NODE *now){
	size_t _data = SIZE_MAX;
	now->FLAGS._I_ = true;
	now->FLAGS._N_ = true;
	for(++now->cur_token; now->cur_token < now->token_cnt; now->cur_token++){
		if(strcasecmp("@", CUR(now)) == 0){
			now->FLAGS._I_ = false;
		}else if(strcasecmp("#", CUR(now)) == 0){
			now->FLAGS._N_ = false;
		}else if(strcasecmp("X", CUR(now)) == 0){
			now->FLAGS._X_ = true;
		}else{
			_data = now->cur_token;
		}
	}
	return _data;
}

void assembler_pass2_object_print(NODE *now, int data){
	char buff[10] = {0,};
	sprintf(buff, "%02X%01X",
			now->OPCODE->opcode + 2 * now->FLAGS._N_ + now->FLAGS._I_,
			8*now->FLAGS._X_ + 4*now->FLAGS._B_ + 2*now->FLAGS._P_ + now->FLAGS._E_);
	if(!now->FLAGS._E_){
		if(0 <= data){
			sprintf(now->OBJECTCODE, "%s%03X", buff, data);
		}else{
			char t[9] = {0,};
			sprintf(t, "%8X", data);
			sprintf(now->OBJECTCODE, "%s%c%c%c", buff, t[5], t[6], t[7]);
		}
	}else{
		if(0 <= data){
			sprintf(now->OBJECTCODE, "%s%05X", buff, data);
		}else{
			char t[9] = {0,};
			sprintf(t, "%8X", data);
			sprintf(now->OBJECTCODE, "%s%c%c%c%c%c", buff, t[3], t[4], t[5], t[6], t[7]);
		}
	}
}

void assembler_pass2_debug_print(NODE *now){
	sprintf(now->OBJECTCODE, "\t%c %c %c %c %c %c",
			now->FLAGS._N_ ? 'N' : ' ',
			now->FLAGS._I_ ? 'I' : ' ',
			now->FLAGS._X_ ? 'X' : ' ',
			now->FLAGS._B_ ? 'B' : ' ',
			now->FLAGS._P_ ? 'P' : ' ',
			now->FLAGS._E_ ? 'E' : ' ');
}

int assembler_get_value_from_register(char *this){
	if(strcasecmp(this, "A") == 0){
		return 0;
	}else if(strcasecmp(this, "X") == 0){
		return 1;
	}else if(strcasecmp(this, "L") == 0){
		return 2;
	}else if(strcasecmp(this, "B") == 0){
		return 3;
	}else if(strcasecmp(this, "S") == 0){
		return 4;
	}else if(strcasecmp(this, "T") == 0){
		return 5;
	}else if(strcasecmp(this, "F") == 0){
		return 6;
	}else if(strcasecmp(this, "PC") == 0){
		return 8;
	}else if(strcasecmp(this, "SW") == 0){
		return 9;
	}
	return -1;
}

DATA *assembler_get_value_from_symbol_or_not(DOCUMENT *doc, char *this){
	DATA *new = (DATA *)calloc(1, sizeof(DATA));
	list_push_back(doc->datas, &(new->elem));

	SYMBOL *target = NULL;
	if((target = symbol_search(doc->symtab, this)) != NULL){
		new->wanted = target->link->LOCATION_CNT;
		new->where = FromSymbol;
	}else if (IsNumberOnly(this)){
		sscanf(this, "%lu", &new->wanted);
		new->where = Integer;
	}else{
		new->where = FAILED;
	}
	return new;
}

bool assembler_make_lst(DOCUMENT *doc, char *filename){
	Elem *find;
	FILE *fout = fopen(filename, "w");

	for(find = list_begin(doc->nodes);
		find != list_end(doc->nodes);
		find = list_next(find)){
			NODE *now = (doc->cur_node = list_entry(find, NODE, elem));
			fprintf(fout, "%5lu\t", now->LINE_NUM); 
			if(!now->FLAGS.COMMENTED_SO_JMP_LST){
				fprintf(fout, "%04X", (unsigned int)(now->LOCATION_CNT)); 
			}
			fprintf(fout, "\t%s\t", now->token_orig); 
			if(now->STORED_DATA != NULL)
				sprintf(now->OBJECTCODE, "%s", now->STORED_DATA);
			fprintf(fout, "%s", now->OBJECTCODE);
			fprintf(fout, "\n");
	}
	doc->cur_node = NULL;
	fclose(fout);
	return false;
}

void assembler_make_obj(DOCUMENT *doc, char *filename){
	Elem *find;
	FILE *fout = fopen(filename, "w");
	size_t cnt = 0, max_cnt = 60;
	Elem *line_from = NULL, *line_end = NULL;
	bool is_storing = false;

	fprintf(fout, "H%-6s%06X%06X\n", doc->progname, (unsigned int)doc->start_addr, (unsigned int)(doc->prev_locctr - doc->start_addr));

	for(find = list_begin(doc->nodes);
		find != list_end(doc->nodes);
		find = list_next(find)){
			NODE *now = (doc->cur_node = list_entry(find, NODE, elem));
			if(now->FLAGS.RESERVED_SO_JMP_OBJ){
				if(is_storing){
					is_storing = false;
					assembler_obj_range_print(fout, line_from, line_end, cnt);
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
						assembler_obj_range_print(fout, line_from, line_end, cnt);
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
	if(is_storing){
		is_storing = false;
		assembler_obj_range_print(fout, line_from, line_end, cnt);
	}
	doc->cur_node = NULL;
	
	for(find = list_begin(doc->modtab);
		find != list_end(doc->modtab);
		find = list_next(find)){
			MODIFY *now = list_entry(find, MODIFY, elem);
			char buff[81] = {0,};
			sprintf(buff, "M%06X%02X", now->target->LOCATION_CNT + 1, 5);
			if(now->more == NULL){
				fprintf(fout, "%s\n", buff);
			}else{
				;  // TODO
			}
	}

	fprintf(fout, "E%06X\n", (unsigned int)doc->end_addr);

	fclose(fout);
}

void assembler_obj_range_print(FILE *fp, Elem *start, Elem *end, size_t cnt){
	Elem *find;
	NODE *one = list_entry(start, NODE, elem);
	if(cnt){
		fprintf(fp, "T%06X%02X", (unsigned int)one->LOCATION_CNT, (unsigned int)cnt/2);

		for(find = start;
			find != list_next(end);
			find = list_next(find)){
				NODE *now = list_entry(find, NODE, elem);
				fprintf(fp, "%s", now->OBJECTCODE);
		}
		fprintf(fp, "\n");
	}
}

DOCUMENT *document_alloc(){
	DOCUMENT *new = (DOCUMENT *)calloc(1, sizeof(DOCUMENT));
	new->nodes = (List *)calloc(1, sizeof(List));
	list_init(new->nodes);
	new->symtab = (List *)calloc(1, sizeof(List));
	list_init(new->symtab);
	new->datas = (List *)calloc(1, sizeof(List));
	list_init(new->datas);
	new->modtab = (List *)calloc(1, sizeof(List));
	list_init(new->modtab);
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

	for(find = list_begin(doc->datas);
		find != list_end(doc->datas);
		/* Do Nothing */){
			DATA *s = list_entry(find, DATA, elem);
			find = list_next(find);
			free(s);
	}
	free(doc->datas);

	for(find = list_begin(doc->modtab);
		find != list_end(doc->modtab);
		/* Do Nothing */){
			MODIFY *s = list_entry(find, MODIFY, elem);
			find = list_next(find);
			if(s->more)
				free(s->more);
			free(s);
	}
	free(doc->modtab);

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

// XXX : For linking issue between *assembler*.c and *assembler*.h, merging all files to one assembler.c file.
#include "modules/assembler_directives.c"
#include "modules/assembler_symbol.c"
#include "modules/disassembler.c"
