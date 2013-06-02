#include "modules/linking_loader.h"
#include "modules/tokenizer.h"
#include "modules/memory.h"
#include <string.h>

#ifdef linking_loader_test
#include "modules/memory.c"
int main(){
	return 0;
}
#endif

bool linking_loader_set_progaddr(char *wanted, size_t *target){
	*target = hex2int(wanted);
	// XXX : IF HAD NEW CONDITION, ADD HERE!
	return true;
}

bool linking_loader_loader(char *filename, void *memory, List *loads, size_t *progaddr, List *modq){
	FILE *fp = fopen(filename, "r");
	if(fp == NULL)
		return false;

	size_t file_base = 0, length;
	bool is_init = false, is_error = false;
	List *ref;
	char *line = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	while((!is_error) && (fgets(line, Tokenizer_Max_Length, fp) != NULL)){
		switch(line[0]){
			case 'H':
			case 'h':
				if(!is_init){
					is_init = true;
					ref = (List *)calloc(1, sizeof(List));
					list_init(ref);
					length = linking_loader_load_header(line, loads, &file_base, *progaddr, ref);
				}else{
					is_error = true;
					printf("No End, before Header.\n");
				}
				break;
			case 'D':
			case 'd':
				if(is_init){
					linking_loader_load_define(line, loads, file_base, *progaddr);
				}else{
					is_error = true;
					printf("No Header, before Define.\n");
				}
				break;
			case 'R':
			case 'r':
				if(is_init){
					linking_loader_load_reserve(line, ref);
				}else{
					is_error = true;
					printf("No Header, before Reserve.\n");
				}
				break;
			case 'T':
			case 't':
				if(is_init){
					linking_loader_load_text(line, memory, file_base, *progaddr);
				}else{
					is_error = true;
					printf("No Header, before Text.\n");
				}
				break;
			case 'M':
			case 'm':
				if(is_init){
					linking_loader_load_modify(line, ref, modq, file_base, *progaddr);
				}else{
					is_error = true;
					printf("No Header, before Modify.\n");
				}
				break;
			case 'E':
			case 'e':
				if(is_init){
					is_init = false;
					*progaddr += length;
					free(ref);
					// TODO Delete REF Clearly.. (not now.. uu)
				}else{
					is_error = true;
					printf("No Header, before End.\n");
				}
				break;
			default:
				break;
		}
		memset(line, 0, Tokenizer_Max_Length);
	}
	free(line);

	fclose(fp);
	return !is_error;
}

size_t linking_loader_load_header(char *in, List *loads, size_t *file_base, size_t progaddr, List *ref){
	size_t length = 0;
	char name[7];
	{
		Load *new = (Load *)calloc(1, sizeof(Load));

		sscanf(in, "H%6s%06X%06X", name, (unsigned int *)file_base, (unsigned int *)&length);

		new->Type = ctrlsec;
		new->Name = strdup(name);
		new->Addr = progaddr;
		new->Length = length;

		list_push_back(loads, &(new->elem));
	}
	{
		Ref *new = (Ref *)calloc(1, sizeof(Ref));
		new->name = strdup(name);
		new->num = 1;
		list_push_back(ref, &(new->elem));
	}
	return length;
}

void linking_loader_load_define(char *in, List *loads, size_t base, size_t progaddr){
	char name[7], *now = in + 1;
	size_t addr;
	while(*now != '\n'){
		Load *new = (Load *)calloc(1, sizeof(Load));
		sscanf(now, "%6s%06X", name, (unsigned int *)&addr);
		addr -= base;
		addr += progaddr;
		new->Type = symbol;
		new->Name = strdup(name);
		new->Addr = addr;
		list_push_back(loads, &(new->elem));
		now += 12;
	}
}

void linking_loader_load_reserve(char *in, List *ref){
	char name[7], *now = in + 1;
	while(*now != '\n'){
		Ref *new = (Ref *)calloc(1, sizeof(Ref));
		sscanf(now, "%02X%6s", (unsigned int *)&new->num, name);
		new->name = strdup(name);
		list_push_back(ref, &(new->elem));
		now += 8;
	}
}

void linking_loader_load_modify(char *in, List *ref, List *modq, size_t base, size_t progaddr){
	char *now = in + 1;
	size_t num;
	ModQueue *new = (ModQueue *)calloc(1, sizeof(ModQueue));
	sscanf(now, "%06X%02X", (unsigned int *)&new->addr, (unsigned int *)&new->bit);
	new->addr -= base;
	new->addr += progaddr;
	now += 8;
	switch(*now){
		case '+':
			new->is_plus = true;
			now += 1;
			sscanf(now, "%02X", (unsigned int *)&num);
			new->target_name = linking_loader_find_ref(num, ref);
			break;
		case '-':
			new->is_plus = false;
			now += 1;
			sscanf(now, "%02X", (unsigned int *)&num);
			new->target_name = linking_loader_find_ref(num, ref);
			break;
		default:
			new->is_plus = true;
			new->target_name = linking_loader_find_ref(1, ref);
			break;
	}
	list_push_back(modq, &(new->elem));
}

char *linking_loader_find_ref(size_t query, List *ref){
	Elem *find;
	for(find = list_begin(ref);
		find != list_end(ref);
		find = list_next(find)){
			Ref *now = list_entry(find, Ref, elem);
			if(now->num == query)
				return now->name;
	}
	return NULL;
}

void linking_loader_fill_modify(void *memory, List *loads, List *modq){
	Elem *find_queue;
	for(find_queue = list_begin(modq);
		find_queue != list_end(modq);
		find_queue = list_next(find_queue)){
			ModQueue *now_queue = list_entry(find_queue, ModQueue, elem);
			Elem *find_load;
			for(find_load = list_begin(loads);
				find_load != list_end(loads);
				find_load = list_next(find_load)){
					Load *now_load = list_entry(find_load, Load, elem);
					if(strcasecmp(now_queue->target_name, now_load->Name) == 0){
						// Let's DO THIS!
						linking_loader_fill_modify_real(memory, now_queue, now_load);
						break;
					}
			}
	}
}

void linking_loader_fill_modify_real(void *memory, ModQueue *nq, Load *nl){
	size_t d = memory_get(memory, nq->addr),
		   e = memory_get(memory, nq->addr + 1),
		   f = memory_get(memory, nq->addr + 2),
		   g = d * 0x10000 + e * 0x100 + f;
	int result = 0;
	if(nq->is_plus){
		result = g + nl->Addr;
	}else{
		result = g - nl->Addr;
	}
	char t[9] = {0,}, s[3] = {0,};
	{
		size_t data = 0;
		sprintf(t, "%8X", result);
		s[0] = t[2];
		s[1] = t[3];
		sscanf(s, "%2X", (unsigned int *)&data);
		memory_edit(memory, nq->addr, data);
	}
	{
		size_t data = 0;
		s[0] = t[4];
		s[1] = t[5];
		sscanf(s, "%2X", (unsigned int *)&data);
		memory_edit(memory, nq->addr + 1, data);
	}
	{
		size_t data = 0;
		s[0] = t[6];
		s[1] = t[7];
		sscanf(s, "%2X", (unsigned int *)&data);
		memory_edit(memory, nq->addr + 2, data);
	}
}

void linking_loader_load_text(char *in, void *memory, size_t base, size_t progaddr){
	size_t start_addr = 0, length = 0;
	char *now = in + 1;
	sscanf(now, "%06X%02X", (unsigned int *)&start_addr, (unsigned int *)&length);
	start_addr -= base;
	start_addr += progaddr;
	now += 8;
	{
		size_t i, data;
		for(i = 0; i < length; i++){
			sscanf(now, "%02X", (unsigned int *)&data);
			memory_edit(memory, start_addr + i, data);
			now += 2;
		}
	}
}

void linking_loader_printer(List *loads){
	printf("\tcontrol\t\tsymbol\t\taddress\t\tlength\n");
	printf("\tsection\t\tname\n");
	printf("\t-----------------------------------------------------------\n");
	size_t total_length = 0;
	{
		Elem *find;
		for(find = list_begin(loads);
				find != list_end(loads);
				find = list_next(find)){
			Load *now = list_entry(find, Load, elem);
			if(now->Type == ctrlsec){
				printf("\t%s\t\t\t\t%04X\t\t%04X\n",
						now->Name, (unsigned int)now->Addr, (unsigned int)now->Length);
				total_length += now->Length;
			}else if(now->Type == symbol){
				printf("\t\t\t%s\t\t%04X\n",
						now->Name, (unsigned int)now->Addr);
			}
		}
	}
	printf("\t-----------------------------------------------------------\n");
	printf("\t\t\t\t\ttotal length\t%04X\n", (unsigned int)total_length);
}

void linking_loader_dealloc(List *loads){
	Elem *find;
	for(find = list_begin(loads);
			find != list_end(loads);
			/* Do Nothing */){
		// TODO : Do more cleaver way by list_remove();
		Load *now = list_entry(find, Load, elem);
		find = list_next(find),
			 free(now->Name);
		free(now);
	}
	list_init(loads);
}
