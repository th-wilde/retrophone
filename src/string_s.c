#include <stdio.h>
#include <string.h>
#include "string_s.h"

void strcat_s(char* str, int length, char *cat){
	if(strlen(str) + strlen(cat) + 1 < length){
		strcat(str, cat);
	}else{
		fprintf(stderr, "error concat oveflow: %s %s > %d", str, cat, length);
	}
}

void strcpy_s(char* str, int length, char *cpy){
	if(strlen(cpy) + 1 < length){
		strcpy(str, cpy);
	}else{
		fprintf(stderr, "error copy oveflow: %s > %d", cpy, length);
	}
}