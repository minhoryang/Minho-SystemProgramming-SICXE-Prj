#include <ctype.h>
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
								bool is_lineNum = true;
								char *n;
								for(n=(doc->cur_node)->token_pass[0]; *n != '\0'; n++){  // LineNum Checker.
									is_lineNum &= (bool)isdigit(*n);
									if(!is_lineNum)
										break;
								}
								if(is_lineNum)
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

bool assembler_pass1(DOCUMENT *doc, Hash *opcode, List *asmdirs){
	// XXX : Pass 1.
	Elem *find;
	bool OMGflag = false;

	for(find = list_begin(doc->nodes);
		find != list_end(doc->nodes);
		find = list_next(find)){

			size_t i = 0;
			bool hasSymbol = false, hasAsmdir = false, hasOpcode = false;
			NODE *now = (doc->cur_node = list_entry(find, NODE, elem));

			for(i = 0; i < now->token_cnt; i++){
				OPMNNode *found_opcode;
				ASMDir *found_asmdir;
				SYMBOL *new_symbol;

				if(!hasSymbol){  // 1. NO SYMBOL
					if(!hasAsmdir && !hasOpcode){  // 1-A. NO SYMBOL & NO CMD
						// need to find 'CMD' or 'SYMBOL'.
#define IF_OPCODE_OR_ASMDIR_DO \
						if((found_opcode = MN_Search(opcode, now->token_pass[i])) != NULL){	\
							if(DEBUG_PRINT)	\
								printf("OPCODE! %s\t", now->token_pass[i]);	\
							hasOpcode = true;	\
							now->OPCODE = found_opcode;	\
							now->_size = 3;	\
						}else if((found_asmdir = assembler_directives_search(asmdirs, now->token_pass[i])) != NULL){	\
							if(DEBUG_PRINT)	\
								printf("ASMDIR! %s\t", now->token_pass[i]);	\
							hasAsmdir = true;	\
							found_asmdir->apply(doc, (void *)now->token_pass[i+1]);	\
						}  // SET AWESOME MACRO for REDUCING DUPLICATED AREA!

						IF_OPCODE_OR_ASMDIR_DO  // if (above)
						else if(strcasecmp(".", now->token_pass[i]) == 0){
							now->FLAGS.COMMENTED_SO_JMP_LST = true;
							break;
						}else{  // 1-A-d. << SHOULD BE SYMBOL >>
							if(DEBUG_PRINT)
								printf("SYMBOL! %s\t", now->token_pass[i]);
							hasSymbol = true;
							if((new_symbol = symbol_add(doc->symtab, now->token_pass[i], now)) != NULL){
								now->Symbol = new_symbol;
								new_symbol = NULL;
							}else{
								printf("ERROR!!!! LINE %lu : RIGHT AFTER SYMBOL's POSITION, NEED TO FOLLOW MNEMONIC/ASMDIRs.\n", now->LINE_NUM);
								OMGflag = true;
							}
						}
					}else{  // NO SYMBOL, GOT CMD.
						if(hasOpcode){  // 1-B. NO SYMBOL, OPCODE WORKS!
							OMGflag = assembler_pass1_got_opcode_check_disp(doc, now, i);	
						}else if(hasAsmdir){  // 1-C. NO SYMBOL, ASMDIR WORKS!
							// XXX : IGNORE for 'END' and 'EQU', ...
						}else{  // 1-D. NO SYMBOL, NO CMD!
							printf("3ERROR!!!! LINE %lu\n", now->LINE_NUM);
							OMGflag = true;
						}
					}
				}else{
					if(!hasAsmdir && !hasOpcode){
						IF_OPCODE_OR_ASMDIR_DO  // if (above)
						else{
							printf("4ERROR!!!! LINE %lu\n", now->LINE_NUM);
							if(DEBUG_PRINT)
								for(i=0;i<now->token_cnt;i++)
									printf("<%s>\n",now->token_pass[i]);
							
							OMGflag = true;
						}
					}else{
						// rest of it would be 'DISP'.
						// --------------------------------
						if(hasOpcode){
							if((OMGflag = assembler_pass1_got_opcode_check_disp(doc, now, i))){
								printf("5ERROR!!!! LINE %lu\n", now->LINE_NUM);
							}
						}else if(hasAsmdir){
							// XXX : IGNORE for 'END' and 'EQU', ...
						}else{
							printf("6ERROR!!!! LINE %lu\n", now->LINE_NUM);
							OMGflag = true;
						}
						// --------------------------------
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

bool assembler_pass1_got_opcode_check_disp(DOCUMENT *doc, NODE *now, size_t i){
	bool OMGflag = false;
	SYMBOL *new_symbol;

	if(strcasecmp("X", now->token_pass[i]) == 0){
		(now->FLAGS)._X_ = true;
		if(DEBUG_PRINT)
			printf("_X_\t");
	}else if(strcasecmp(".", now->token_pass[i]) == 0){
		// IGNORE COMMENT!
	}else if((new_symbol = symbol_add(doc->symtab, now->token_pass[i], NULL)) != NULL){
		now->DISP = new_symbol;
		new_symbol = NULL;
	}else{
		printf("2ERROR!!!! LINE %lu\n", now->LINE_NUM);
		OMGflag = true;
	}
	return OMGflag;
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

void assembler_make_obj(DOCUMENT *doc, char *filename){
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

// XXX : For linking issue between *assembler*.c and *assembler*.h, merging all files to one assembler.c file.
#include "modules/assembler_directives.c"
#include "modules/assembler_symbol.c"
#include "modules/disassembler.c"
