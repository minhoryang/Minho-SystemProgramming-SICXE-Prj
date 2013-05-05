#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "modules/optab.h"

#include "core/hex.h"
#include "core/list.h"
#include "modules/tokenizer.h"

#ifdef optab_test
	#include "modules/tokenizer.c"

	int main(){
		Hash *OP = OP_Alloc(),
			 *MN = MN_Alloc();
		OPMN_Load(OP, MN);
		OP_List(OP);
		MN_List(MN);
		hash_destroy(OP, both_hash_destructor);
		hash_destroy(MN, NULL);
		return 0;
	}
#endif

Hash *OP_Alloc(){
	return hash_init(op_hash_func, op_less_func, NULL);
}

Hash *MN_Alloc(){
	return hash_init(mn_hash_func, mn_less_func, NULL);
}

unsigned int op_hash_func(const struct hash_elem *e, void *aux){
	OPMNNode *wanted = hash_entry(e, OPMNNode, op_elem);
	char s[3] = {0,};
	sprintf(s, "%2X", wanted->opcode);
	unsigned int result = 0;
	size_t i, j;
	for (i = 0; i < 3; i++){
		unsigned int t = 1;
		for(j = 0; j < i; j++){
			t *= 26;
		}
		result += t * (unsigned int)s[i];
	}
	return result % 20;
}

unsigned int mn_hash_func(const struct hash_elem *e, void *aux){
	OPMNNode *wanted = hash_entry(e, OPMNNode, mn_elem);
	size_t s = strlen(wanted->mnemonic), i, j;
	unsigned int result = 0;
	for (i = 0; i < s; i++){
		unsigned int t = 1;
		for(j = 0; j < i; j++){
			t *= 26;
		}
		result += t * (unsigned int)wanted->mnemonic[i];
	}
	return result % 20;
}

void both_hash_destructor(const struct hash_elem *e, void *aux){
	OPMNNode *wanted = hash_entry(e, OPMNNode, op_elem);
	free(wanted);
}


bool op_less_func(
		const struct hash_elem *a,
		const struct hash_elem *b,
		void *aux){
	OPMNNode *node_a = hash_entry(a, OPMNNode, op_elem),
			 *node_b = hash_entry(b, OPMNNode, op_elem);
	if(node_a->opcode < node_b->opcode)
		return true;
	return false;
}

bool mn_less_func(
		const struct hash_elem *a,
		const struct hash_elem *b,
		void *aux){
	OPMNNode *node_a = hash_entry(a, OPMNNode, mn_elem),
			 *node_b = hash_entry(b, OPMNNode, mn_elem);
	if(strcasecmp(node_a->mnemonic, node_b->mnemonic) < 0)
		return true;
	return false;
}

void OPMN_Load(Hash *OP, Hash *MN){
	FILE *fp = fopen("opcode.txt", "r");
	char *line = (char *)calloc(Tokenizer_Max_Length, sizeof(char)), **out;
	size_t cnt;
	while(fgets(line, Tokenizer_Max_Length, fp) != NULL){
		if((cnt = Tokenizer(line, (out = AllocToken()), false))){
			bool formats[4] = {false,};
			{  // OPCODE FORMAT HANDLER!
				int a[4] = {0,},
					n = sscanf(out[2],
							"%d%*c%d%*c%d%*c%d",  // RULES
							&a[0], &a[1], &a[2], &a[3]);
				{
					int i;
					for (i=0;i<n;i++)
						if(1 <= a[i] && a[i] <= 4)
							formats[a[i]-1] = true;
				}
				if(0)  // TODO(DEBUG)
					printf("%lu %s %s (%d : %d %d %d %d)\n",
						cnt, out[0], out[1],
						n, a[0], a[1], a[2], a[3]);
			}
			OPMN_Insert(OP, MN, out[0], out[1], formats);
		}
		DeAllocToken(out);
	}
	free(line);
	fclose(fp);
}

void OPMN_Insert(Hash *OP, Hash *MN, char *opcode, char *mnemonic, bool *formats){
	OPMNNode *new = (OPMNNode *)calloc(1, sizeof(OPMNNode));
	strncpy(new->mnemonic, mnemonic, strlen(mnemonic));
	new->opcode = hex2int(opcode);
	if(0)  // TODO(DEBUG)
		printf("%s %s %s %s\n",
			formats[0] ? "true" : "false",
			formats[1] ? "true" : "false",
			formats[2] ? "true" : "false",
			formats[3] ? "true" : "false");
	new->can_format[0] = formats[0];
	new->can_format[1] = formats[1];
	new->can_format[2] = formats[2];
	new->can_format[3] = formats[3];
	if(OP != NULL)
		hash_insert(OP, &new->op_elem);
	if(MN != NULL)
		hash_insert(MN, &new->mn_elem);
}

void OP_List(Hash *what){
	size_t i, t = 1;
	for(i = 0; i < what->bucket_cnt; i++){
		if(i && (i % 4 == 0))  printf("%lu : \n", i + t++);
		printf("%lu : ", i + t);
		{
			bool arrow = false;
			List *now = &what->buckets[i];
			Elem *find;
			for(find = list_begin(now);
				find != list_end(now);
				find = list_next(find)){
				if(arrow)
					printf(" -> ");
				arrow = true;
				HElem *this = Elem2HElem(find);
				OPMNNode *realthis = hash_entry(this, OPMNNode, op_elem);
				printf("[%s,%2X]", realthis->mnemonic, realthis->opcode);
			}
		}
		printf("\n");
	}
	printf("%lu : \n", i + t++);
}

void MN_List(Hash *what){
	size_t i, t = 1;
	for(i = 0; i < what->bucket_cnt; i++){
		if(i && (i % 4 == 0))  printf("%lu : \n", i + t++);
		printf("%lu : ", i + t);
		{
			bool arrow = false;
			List *now = &what->buckets[i];
			Elem *find;
			for(find = list_begin(now);
				find != list_end(now);
				find = list_next(find)){
				if(arrow)
					printf(" -> ");
				arrow = true;
				HElem *this = Elem2HElem(find);
				OPMNNode *realthis = hash_entry(this, OPMNNode, mn_elem);
				printf("[%s,%2X]", realthis->mnemonic, realthis->opcode);
			}
		}
		printf("\n");
	}
	printf("%lu : \n", i + t++);
}

OPMNNode *MN_Search(Hash *what, char *mnemonic){
	size_t i;
	for(i = 0; i < what->bucket_cnt; i++){
		{
			List *now = &what->buckets[i];
			Elem *find;
			for(find = list_begin(now);
				find != list_end(now);
				find = list_next(find)){
				HElem *this = Elem2HElem(find);
				OPMNNode *realthis = hash_entry(this, OPMNNode, op_elem);
				if(strcasecmp(realthis->mnemonic, mnemonic) == 0){  // XXX FIXED : Added lower.
					return realthis;  // XXX FIXED : return OPMNNode *.
				}
			}
		}

	}
	return NULL;
}

OPMNNode *OP_Search(Hash *what, char *opcode){
	size_t i;
	for(i = 0; i < what->bucket_cnt; i++){
		{
			List *now = &what->buckets[i];
			Elem *find;
			for(find = list_begin(now);
				find != list_end(now);
				find = list_next(find)){
				HElem *this = Elem2HElem(find);
				OPMNNode *realthis = hash_entry(this, OPMNNode, mn_elem);
				if(realthis->opcode == hex2int(opcode)){
					return realthis;  // XXX FIXED : return OPMNNode *.
				}
			}
		}
	}
	return NULL;
}

