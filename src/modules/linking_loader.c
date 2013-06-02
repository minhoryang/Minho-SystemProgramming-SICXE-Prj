#include "modules/linking_loader.h"
#include <string.h>

#ifdef linking_loader_test
	int main(){
		return 0;
	}
#endif

bool linking_loader_set_progaddr(char *wanted, size_t *target){
	*target = hex2int(wanted);
	// XXX : IF HAD NEW CONDITION, ADD HERE!
	return true;
}

bool linking_loader_loader(char *filename, void *memory, List *loads, size_t *progaddr){
	FILE *fp = fopen(filename, "r");
	if(fp == NULL)
		return false;

	Load *new = (Load *)calloc(1, sizeof(Load));
	new->Type = ctrlsec;
	new->Name = strdup(filename);
	list_push_back(loads, &(new->elem));

	fclose(fp);
	return true;
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
					printf("\t%s\t\t\t%04lu\t\t%04lu\n",
							now->Name, now->Addr, now->Length);
					total_length += now->Length;
				}else if(now->Type == symbol){
					printf("\t\t\t%s\t%04lu\n",
							now->Name, now->Addr);
				}
		}
	}
	printf("\t-----------------------------------------------------------\n");
	printf("\t\t\t\t\ttotal length\t%04lu\n", total_length);
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
