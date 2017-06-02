#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "popen2.h"

static FILE* espeek_pipe_handle;
static pid_t espeak_pid;

void rpsay_init(){
	int espeak_pipe_in;
	int espeak_pipe_out;
	char* command[] = {"espeak", "-v", "de", NULL};
	espeak_pid = popen2(command, &espeak_pipe_in, &espeak_pipe_out);
	close(espeak_pipe_out);
	espeek_pipe_handle = fdopen(espeak_pipe_in, "w");
}
void rpsay_char(char character){
	
	fprintf(espeek_pipe_handle, "%c.\n\n", character);
	fflush(espeek_pipe_handle);
}
void rpsay_string(char* text){
	fprintf(espeek_pipe_handle, "%s.\n\n", text);
	fflush(espeek_pipe_handle);
}

void rpsay_spell(char* text){
	
	for(int i=0; i<strlen(text); i++){
		switch(text[i]){
			case 'A':
				rpsay_string("Großes A");
				break;
			case 'B':
				rpsay_string("Großes Be");
				break;
			case 'C':
				rpsay_string("Großes Ce");
				break;
			case 'D':
				rpsay_string("Großes De");
				break;
			case 'E':
				rpsay_string("Großes E");
				break;
			case 'F':
				rpsay_string("Großes Ef");
				break;
			case 'G':
				rpsay_string("Großes Ge");
				break;
			case 'H':
				rpsay_string("Großes Ha");
				break;
			case 'I':
				rpsay_string("Großes I");
				break;
			case 'J':
				rpsay_string("Großes Jot");
				break;
			case 'K':
				rpsay_string("Großes Ka");
				break;
			case 'L':
				rpsay_string("Großes El");
				break;
			case 'M':
				rpsay_string("Großes Em");
				break;
			case 'N':
				rpsay_string("Großes En");
				break;
			case 'O':
				rpsay_string("Großes O");
				break;
			case 'P':
				rpsay_string("Großes Pe");
				break;
			case 'Q':
				rpsay_string("Großes Q");
				break;
			case 'R':
				rpsay_string("Großes Er");
				break;
			case 'S':
				rpsay_string("Großes Es");
				break;
			case 'T':
				rpsay_string("Großes Te");
				break;
			case 'U':
				rpsay_string("Großes U");
				break;
			case 'V':
				rpsay_string("Großes Vau");
				break;
			case 'W':
				rpsay_string("Großes We");
				break;
			case 'X':
				rpsay_string("Großes Ix");
				break;
			case 'Y':
				rpsay_string("Großes Ypsilon");
				break;
			case 'Z':
				rpsay_string("Großes Zet");
				break;
			case 'a':
				rpsay_string("Kleines A");
				break;
			case 'b':
				rpsay_string("Kleines Be");
				break;
			case 'c':
				rpsay_string("Kleines Ce");
				break;
			case 'd':
				rpsay_string("Kleines De");
				break;
			case 'e':
				rpsay_string("Kleines E");
				break;
			case 'f':
				rpsay_string("Kleines Ef");
				break;
			case 'g':
				rpsay_string("Kleines Ge");
				break;
			case 'h':
				rpsay_string("Kleines Ha");
				break;
			case 'i':
				rpsay_string("Kleines I");
				break;
			case 'j':
				rpsay_string("Kleines Jot");
				break;
			case 'k':
				rpsay_string("Kleines Ka");
				break;
			case 'l':
				rpsay_string("Kleines El");
				break;
			case 'm':
				rpsay_string("Kleines Em");
				break;
			case 'n':
				rpsay_string("Kleines En");
				break;
			case 'o':
				rpsay_string("Kleines O");
				break;
			case 'p':
				rpsay_string("Kleines Pe");
				break;
			case 'q':
				rpsay_string("Kleines Q");
				break;
			case 'r':
				rpsay_string("Kleines Er");
				break;
			case 's':
				rpsay_string("Kleines Es");
				break;
			case 't':
				rpsay_string("Kleines Te");
				break;
			case 'u':
				rpsay_string("Kleines U");
				break;
			case 'v':
				rpsay_string("Kleines Vau");
				break;
			case 'w':
				rpsay_string("Kleines We");
				break;
			case 'x':
				rpsay_string("Kleines Ix");
				break;
			case 'y':
				rpsay_string("Kleines Ypsilon");
				break;
			case 'z':
				rpsay_string("Kleines Zet");
				break;
			case '0':
				rpsay_string("Null");
				break;
			case '1':
				rpsay_string("Eins");
				break;
			case '2':
				rpsay_string("Zwei");
				break;
			case '3':
				rpsay_string("Drei");
				break;
			case '4':
				rpsay_string("Vier");
				break;
			case '5':
				rpsay_string("Fünf");
				break;
			case '6':
				rpsay_string("Sechs");
				break;
			case '7':
				rpsay_string("Sieben");
				break;
			case '8':
				rpsay_string("Acht");
				break;
			case '9':
				rpsay_string("Neun");
				break;
			default:
				rpsay_string("Unbekantes Zeichen");
		}
	}
}

void rpsay_quit(){
	fclose(espeek_pipe_handle);
	kill (espeak_pid, SIGTERM);
}