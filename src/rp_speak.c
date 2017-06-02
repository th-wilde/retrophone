#include <stdio.h>
#include <unistd.h>

static FILE* espeek_pipe;

void rpsay_init(){
	espeek_pipe = popen("espeak -v de 2>/dev/null", "w");
}
void rpsay_char(char character){
	printf("Send %c\n\n", character);
	fprintf(espeek_pipe, "%c.\n\n", character);
	fflush(espeek_pipe);
}
void rpsay_string(char* text){
	fprintf(espeek_pipe, "%s\n\n", text);
	fflush(espeek_pipe);
}

void rpsay_quit(){
	pclose(espeek_pipe);
}