#include <stdio.h>
#include <stdlib.h>

#include "core/stringswitch.h"

#ifdef stringswitch_test
int main(){
	char *list[] = {"hello", "world"};
	StringSwitchSet *a = AllocStringSwitchSet(list, 2);
	printf("%d\n", String2Switch(a, "hello"));
	printf("%d\n", String2Switch(a, "world"));
	printf("%d\n", String2Switch(a, "fuck"));
	StringSwitch(a, "hello"){
		case 0:
			printf("pass\n");
			break;
		default:
			printf("fail\n");
			break;
	}
	DeAllocStringSwitchSet(a);
	return 0;
}
#endif


StringSwitchSet *AllocStringSwitchSet(char *list[], size_t length){
	StringSwitchSet *a = (StringSwitchSet *)calloc(1, sizeof(StringSwitchSet));
	a->length = length;
	a->list = (char **)calloc(length, sizeof(char *));
	{
		size_t i;
		for(i = 0; i < a->length; i++)
			a->list[i] = list[i];
	}
	return a;
}

void DeAllocStringSwitchSet(StringSwitchSet *a){
	/*{
		size_t i;
		for(i = 0; i< a->length; i++)
			free(a->list[i]);
	}*/
	free(a->list);
	free(a);
}

int String2Switch(StringSwitchSet *a, char * const input){
	int n;
	for(n = a->length-1; n >= 0; n--){
		if(!strcmp(a->list[n], input)){
			break;
		}
	}
	return n;
}
