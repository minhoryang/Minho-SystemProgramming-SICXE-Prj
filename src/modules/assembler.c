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
		bool select_test = true;
		if(select_test){
			char *filename = "2-15.asm",   // TODO FILENAME
				 *filename1 = strdup(filename),
				 *filename2 = strdup(filename);
			strcat(filename1, ".lst");
			strcat(filename2, ".obj");

			DOCUMENT *doc;

			Hash *OP = OP_Alloc();  // OPCode Load.
			OPMN_Load(OP, NULL, NULL);

			List *directives = assembler_directives_load();
		
			assembler_readline(filename, (doc = document_alloc()));
			assembler_pass1(doc, OP, directives);
			assembler_pass2(doc);
			assembler_make_lst(doc, filename1);
			assembler_make_obj(doc, filename2);

			hash_destroy(OP, both_hash_destructor);  // OPCode Unload.
			assembler_directives_unload(directives);
			document_dealloc(doc);
		}else{
			Hash *OP = OP_Alloc();  // OPCode Load.
			Hash *MN = MN_Alloc();
			OPMN_Load(OP, MN, NULL);
			// disassembler("objobj", MN);
			hash_destroy(OP, both_hash_destructor);  // OPCode Unload.
			hash_destroy(MN, NULL);
		}
		return 0;
	}
#endif

bool assembler_readline(char *filename, DOCUMENT *doc){
	// XXX : Read Line -> Assign Node -> set$lineNum$ -> Add Node @ CSECT.
	FILE *fin = fopen(filename, "r");
	size_t lineNum = 0, jmp = 5, blockNum = 0;
	enum ThreeStates {reset=-1, no, yes} lineNumIn = reset;

	if(fin == NULL) return true;

	// ADD (default) Csect to Document!
	CSECT *csect = (doc->cur_csect = csect_alloc());
	list_push_back(doc->csects, &(csect->elem));
	csect->_PARENT = doc;

	// Add (default) Block to Csect!
	csect->cur_block = block_alloc(blockNum++, "(default)");
	list_push_back(csect->blocks, &(csect->cur_block->elem));
	csect->cur_block->_PARENT = csect;
	NODE *nd = NULL;

	while(
		fgets(  // Allocate 1 Node and Get 1 line.
			(nd = node_alloc())->token_orig,
			Tokenizer_Max_Length,
			fin) != NULL){
				{  // Pass tokenier for 'Enter' and Swap these.
					char *new = Tokenizer_NoEnter(nd->token_orig);
					free(nd->token_orig);
					nd->token_orig = new;
				}
				{  // Tokenizer!
					nd->token_cnt = Tokenizer(
									nd->token_orig,
									nd->token_pass,
									SIC);
				}
				{  // Find 'CSECT' ASMDIR, and SWITCH IT.
					// if CSECT DETECT!
					 // make new csect+block! -> switch!
					char *newcsect;
					if((newcsect = document_csect_detect(nd)) != NULL){
						{
							CSECT *new = (doc->cur_csect = csect_alloc());
							list_push_back(doc->csects, &(new->elem));
							new->_PARENT = doc;
							csect = new;
						}
						{
							blockNum = 0;
							BLOCK *new = (doc->cur_csect->cur_block = block_alloc(blockNum++, "(default)"));
							list_push_back(doc->cur_csect->blocks, &(new->elem));
							new->_PARENT = doc->cur_csect;
						}
					}
				}
				{  // Find 'USE' ASMDIR, and SWITCH IT.
					char *newuse;
					if((newuse = block_detect(nd)) != NULL){
						BLOCK *found;
						if((found = block_search(csect->blocks, newuse)) != NULL){
							csect->cur_block = found;
							
								// if has -> switch
						}else{
							csect->cur_block = block_alloc(blockNum++, newuse);
							list_push_back(csect->blocks, &(csect->cur_block->elem));
							csect->cur_block->_PARENT = csect;
								// if not -> make & switch.
						}
					}
				}
				{  // Add (nd) Node to (?) Block
					list_push_back(csect->cur_block->nodes, &(nd->elem));
					csect->cur_block->cur_node = nd;
					nd->_PARENT = csect->cur_block;
				}
				{  // XXX : LineNumber Generator.
					char *swap;
					if(lineNumIn == reset){  // ONE-TIME-ONLY. Try to find LineNum at inputs.
						if((csect->cur_block->cur_node)->token_cnt > 0){
							if(IsNumberOnly((csect->cur_block->cur_node)->token_pass[0]))
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
							(csect->cur_block->cur_node)->LINE_NUM = (unsigned int)atoi((csect->cur_block->cur_node)->token_pass[0]);
							if((csect->cur_block->cur_node)->token_cnt)
							{  // IGNORE LINENUMBER FOR ASSEMBLER_PASS.
								{  // Shift << 'LineNum'.
									swap = (csect->cur_block->cur_node)->token_pass[0];  // 0
									for(jmp = 1; jmp < Tokenizer_Max_Length; jmp++)  // 1~81=>0~80
										(csect->cur_block->cur_node)->token_pass[jmp-1] = (csect->cur_block->cur_node)->token_pass[jmp];
									(csect->cur_block->cur_node)->token_pass[Tokenizer_Max_Length-1] = swap;  // 81=>0.
								}
								(csect->cur_block->cur_node)->token_cnt--;  // Set 'ignore' as decreasing token_cnt.
							}
							break;
						case no:
							lineNum = ((csect->cur_block->cur_node)->LINE_NUM = lineNum + jmp);
							break;
					}
		}
	}
	csect->cur_block->cur_node = NULL;
	csect->cur_block = NULL;
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
	bool OMGflag = false;

	Elem *find_csect;
	for(find_csect = list_begin(doc->csects);
		find_csect != list_end(doc->csects);
		find_csect = list_next(find_csect)){

		CSECT *csect = (doc->cur_csect = list_entry(find_csect, CSECT, elem));
		Elem *find_block;
	
		for(find_block = list_begin(csect->blocks);
			find_block != list_end(csect->blocks);
			find_block = list_next(find_block)){
	
			BLOCK *blk = (csect->cur_block = list_entry(find_block, BLOCK, elem));
			blk->BASE = (blk->prev_locctr = csect->prev_base);
			Elem *find_node;
	
			for(find_node = list_begin(blk->nodes);
				find_node != list_end(blk->nodes);
				find_node = list_next(find_node)){
		
					bool hasSymbol = false, hasAsmdir = false, hasOpcode = false;
					NODE *now = (blk->cur_node = list_entry(find_node, NODE, elem));
		
					now->Literal = literal_detect(csect);
		
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
									if((new_symbol = symbol_add(csect->symtab, CUR(now), now)) != NULL){
										now->Symbol = new_symbol;
										new_symbol = NULL;
									}else{
										printf("ERROR!!!! LINE %lu : RIGHT AFTER SYMBOL's POSITION, NEED TO FOLLOW MNEMONIC/ASMDIRs.\n", now->LINE_NUM);
										OMGflag = true;
										break;
									}
								}
							}
						}else{
							if(!hasAsmdir && !hasOpcode){
								IF_OPCODE_OR_ASMDIR_DO  // if (above)
								else{
									printf("ERROR!! NO ASMDIR&OPCODE LINE %lu\n", now->LINE_NUM);
									size_t i = 0;
									if(DEBUG_PRINT)
										for(i=0;i<now->token_cnt;i++)
											printf("<%s>\n",now->token_pass[i]);
									
									OMGflag = true;
									break;
								}
							}
						}
					}
					now->LOCATION_CNT = blk->prev_locctr;
					blk->prev_locctr = now->LOCATION_CNT + now->_size;
					if(DEBUG_PRINT)
						printf(" %04X\n", (unsigned int)now->LOCATION_CNT);
					if(OMGflag)
						break;
			}
			csect->prev_base = blk->prev_locctr;
			blk->SIZE = blk->prev_locctr - blk->BASE;
			blk->cur_node = NULL;
			if(OMGflag)
				break;
		}
		csect->cur_block = NULL;
	}
	doc->cur_csect = NULL;
	return OMGflag;
}

bool assembler_pass2(DOCUMENT *doc){
	// XXX : Pass 2.
	bool OMGflag = false;

	Elem *find_csect;
	for(find_csect = list_begin(doc->csects);
		find_csect != list_end(doc->csects);
		find_csect = list_next(find_csect)){

		CSECT *csect = (doc->cur_csect = list_entry(find_csect, CSECT, elem));
		Elem *find_block;

		if(!assembler_directives_BASE_TO_BE(doc, false))
			printf("ERR! couldn't calc BASE\n");
	
		for(find_block = list_begin(csect->blocks);
			find_block != list_end(csect->blocks);
			find_block = list_next(find_block)){

			BLOCK *blk = (csect->cur_block = list_entry(find_block, BLOCK, elem));
			Elem *find_node;
	
			for(find_node = list_begin(blk->nodes);
				find_node != list_end(blk->nodes);
				find_node = list_next(find_node)){
	
					NODE *now = (blk->cur_node = list_entry(find_node, NODE, elem));
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
											printf("ERR! FORMAT2, no value? LINE NUM %lu\n", now->LINE_NUM);
											OMGflag = true;
											break;
									}
									sprintf(now->OBJECTCODE, "%02X%01X%01X", now->OPCODE->opcode, a, b);
								}
								break;
							case 4:
								now->FLAGS._E_ = true;
								// XXX : Exception - [MODIFICATION]
#define F4_DISP_COND			if(now->FLAGS._E_){ \
									; \
								}
#define F4_EXC_MODIFY			if(now->FLAGS._E_){ \
									if(value->where == Symbol){ \
										MODIFY *md = (MODIFY *)calloc(1, sizeof(MODIFY)); \
										md->target = now; \
										md->more = NULL; \
										list_push_back(csect->modtab, &(md->elem)); \
									}else if(value->where == Shared){ \
										MODIFY *md = (MODIFY *)calloc(1, sizeof(MODIFY)); \
										md->target = now; \
										md->more = (struct _MODIFY_DATA *)calloc(1, sizeof(struct _MODIFY_DATA)); \
										md->more->is_plus = true; \
										md->more->this = strdup(CUR2(now, -1)); \
										list_push_back(csect->modtab, &(md->elem)); \
									} \
								}
							case 3:
								{
									DATA *value = NULL;
									if(now->Literal == NULL){
										size_t need_to_find = assembler_pass2_set_flag(now);
										if(need_to_find != SIZE_MAX)
											value = assembler_get_value_from_symbol_or_not(csect, now->token_pass[need_to_find]);
									}else{
											assembler_pass2_set_flag(now);
										value = (DATA *)calloc(1, sizeof(DATA));
										value->where = Literal;
										value->wanted = now->Literal->where->LOCATION_CNT;
									}
									{
										int disp = 0;
										if(value){
											if(value->where != FAILED){
												disp = value->wanted;
												if(value->where != Integer){
													int PC = (now->LOCATION_CNT + now->_size);
													if((-2048 <= disp - PC) && (disp - PC <= 2047)){
														disp -= PC;
														now->FLAGS._P_ = true;
													}else if((csect->is_base == Set) && (0 <= disp - csect->base) && (disp - csect->base <= 4095)){
														disp -= csect->base;
														now->FLAGS._B_ = true;
													}else F4_DISP_COND  // XXX : DISP @ FORMAT 4.
													else{
														printf("ERR! FORMAT3/4, FAILED TO CALC DISP! LINE NUM %lu\n", now->LINE_NUM);
														OMGflag = true;
														break;
														//assembler_pass2_debug_print(now);
													}
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
					if(OMGflag)
						break;
			}
			blk->cur_node = NULL;
			if(OMGflag)
				break;
		}
		csect->cur_block = NULL;
		if(OMGflag)
			break;
	}
	doc->cur_csect= NULL;
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

DATA *assembler_get_value_from_symbol_or_not(CSECT *csect, char *this){
	DATA *new = (DATA *)calloc(1, sizeof(DATA));
	list_push_back(csect->datas, &(new->elem));

	SYMBOL *target = NULL;
	SHARED *shr = NULL;
	if((shr = shared_search(csect->_PARENT->shared, this)) != NULL){
		new->where = Shared;
		new->wanted = 0;
		// TODO
	}else if((target = symbol_search(csect->symtab, this)) != NULL){
		if(!target->is_equ){
			new->wanted = target->link->LOCATION_CNT;
			new->where = Symbol;
		}else{
			new->wanted = target->equ;
			new->where = Integer;
		}
	}else if (IsNumberOnly(this)){
		sscanf(this, "%lu", &new->wanted);
		new->where = Integer;
	}else{
		new->where = FAILED;
	}
	return new;
}

bool assembler_make_lst(DOCUMENT *doc, char *filename){
	FILE *fout = fopen(filename, "w");
	Elem *find_csect;
	for(find_csect = list_begin(doc->csects);
		find_csect != list_end(doc->csects);
		find_csect = list_next(find_csect)){

		CSECT *csect = (doc->cur_csect = list_entry(find_csect, CSECT, elem));
		Elem *find_block;

		for(find_block = list_begin(csect->blocks);
			find_block != list_end(csect->blocks);
			find_block = list_next(find_block)){
	
			BLOCK *blk = (csect->cur_block = list_entry(find_block, BLOCK, elem));
			Elem *find_node;
	
			for(find_node = list_begin(blk->nodes);
				find_node != list_end(blk->nodes);
				find_node = list_next(find_node)){
					NODE *now = (blk->cur_node = list_entry(find_node, NODE, elem));
					if(now->LINE_NUM != 0)
						fprintf(fout, "%5lu", now->LINE_NUM); 
					fprintf(fout, "\t"); 
					if(!now->FLAGS.COMMENTED_SO_JMP_LST){
						if(now->Symbol != NULL && now->Symbol->is_equ){
							fprintf(fout, "%04X", (unsigned int)(now->Symbol->equ));
						}else{
							fprintf(fout, "%04X", (unsigned int)(now->LOCATION_CNT - now->_PARENT->BASE));
						}
						fprintf(fout, " %lu", now->_PARENT->ID);
					}
					fprintf(fout, "\t%s\t", now->token_orig); 
					if(now->STORED_DATA != NULL)
						sprintf(now->OBJECTCODE, "%s", now->STORED_DATA);
					fprintf(fout, "%s", now->OBJECTCODE);
					fprintf(fout, "\n");
			}
			blk->cur_node = NULL;
		}
		csect->cur_block = NULL;
	}
	doc->cur_csect = NULL;
	fclose(fout);
	return false;
}

void assembler_make_obj(DOCUMENT *doc, char *filename){
	FILE *fout = fopen(filename, "w");

	Elem *find_csect;
	for(find_csect = list_begin(doc->csects);
		find_csect != list_end(doc->csects);
		find_csect = list_next(find_csect)){

		CSECT *csect = (doc->cur_csect = list_entry(find_csect, CSECT, elem));
		size_t cnt = 0, max_cnt = 60;
		Elem *line_from = NULL, *line_end = NULL;
		bool is_storing = false;

		fprintf(fout, "H%-6s%06X%06X\n", csect->progname, (unsigned int)csect->start_addr, (unsigned int)(csect->prev_base - csect->start_addr));
	
		{
			bool print_once = true;
			Elem *find_define;
			for(find_define = list_begin(csect->define);
				find_define != list_end(csect->define);
				find_define = list_next(find_define)){
				if(print_once){
					print_once = false;
					fprintf(fout, "D");
				}
				DEFINE *def = list_entry(find_define, DEFINE, elem);
				fprintf(fout, "%-6s%06X", def->symbol, (unsigned int)(assembler_get_value_from_symbol_or_not(csect, def->symbol))->wanted);
			}
			if(!print_once)
				fprintf(fout, "\n");
		}
		{
			bool print_once = true;
			Elem *find_define;
			for(find_define = list_begin(csect->reference);
				find_define != list_end(csect->reference);
				find_define = list_next(find_define)){
				if(print_once){
					print_once = false;
					fprintf(fout, "R");
				}
				DEFINE *def = list_entry(find_define, DEFINE, elem);
				fprintf(fout, "%-6s", def->symbol);
			}
			if(!print_once)
				fprintf(fout, "\n");
		}

		Elem *find_block;
		for(find_block = list_begin(csect->blocks);
			find_block != list_end(csect->blocks);
			find_block = list_next(find_block)){
	
			BLOCK *blk = (csect->cur_block = list_entry(find_block, BLOCK, elem));
			Elem *find_node;
	
			for(find_node = list_begin(blk->nodes);
				find_node != list_end(blk->nodes);
				find_node = list_next(find_node)){
					NODE *now = (blk->cur_node = list_entry(find_node, NODE, elem));
					if(now->FLAGS.RESERVED_SO_JMP_OBJ){
						if(is_storing){
							is_storing = false;
							assembler_obj_range_print(fout, line_from, line_end, cnt);
						}
					}else{
						if(!is_storing){
							is_storing = true;
							line_from = find_node;
							line_end = find_node;
							cnt = strlen(now->OBJECTCODE);
						}else{
							if(cnt + strlen(now->OBJECTCODE) > max_cnt){
								is_storing = false;
								assembler_obj_range_print(fout, line_from, line_end, cnt);
								is_storing = true;
								line_from = find_node;
								line_end = find_node;
								cnt = strlen(now->OBJECTCODE);
							}else{
								line_end = find_node;
								cnt += strlen(now->OBJECTCODE);
							}
						}
					}
			}
			if(is_storing){
				is_storing = false;
				assembler_obj_range_print(fout, line_from, line_end, cnt);
			}
			blk->cur_node = NULL;
		}
		csect->cur_block = NULL;

		Elem *find;
		for(find = list_begin(csect->modtab);
			find != list_end(csect->modtab);
			find = list_next(find)){
				MODIFY *now = list_entry(find, MODIFY, elem);
				char buff[81] = {0,};
				sprintf(buff, "M%06X",
						(unsigned int)now->target->LOCATION_CNT + 1);
				if(now->more == NULL){
					fprintf(fout, "%s%02X\n", buff, 5);
				}else{
					fprintf(fout, "%s%02X%c%s\n", buff, 6,
							now->more->is_plus ? '+' : '-', now->more->this);
				}
		}
	
		fprintf(fout, "E%06X\n", (unsigned int)csect->end_addr);
	}
	doc->cur_csect = NULL;
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
#include "modules/assembler_literal.c"
#include "modules/assembler_block.c"
#include "modules/assembler_csect.c"
#include "modules/assembler_document.c"
