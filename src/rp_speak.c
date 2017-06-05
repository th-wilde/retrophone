#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "popen2.h"
#include "string_s.h"

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

void rpsay_string(char* text){
	fprintf(espeek_pipe_handle, "%s.\n\n", text);
	fflush(espeek_pipe_handle);
}

void rpsay_spell(char* text){
	
	int spell_string_buffer = 1024;
	char spell_string[spell_string_buffer];
	spell_string[0] = 0x00;
	
	for(int i=0; i<strlen(text); i++){
		switch(text[i]){
			case 'A':
				strcat_s(spell_string, spell_string_buffer, "Großes A, ");
				break;
			case 'B':
				strcat_s(spell_string, spell_string_buffer, "Großes Be, ");
				break;
			case 'C':
				strcat_s(spell_string, spell_string_buffer, "Großes Ce, ");
				break;
			case 'D':
				strcat_s(spell_string, spell_string_buffer, "Großes De, ");
				break;
			case 'E':
				strcat_s(spell_string, spell_string_buffer, "Großes E, ");
				break;
			case 'F':
				strcat_s(spell_string, spell_string_buffer, "Großes Ef, ");
				break;
			case 'G':
				strcat_s(spell_string, spell_string_buffer, "Großes Ge, ");
				break;
			case 'H':
				strcat_s(spell_string, spell_string_buffer, "Großes Ha, ");
				break;
			case 'I':
				strcat_s(spell_string, spell_string_buffer, "Großes I, ");
				break;
			case 'J':
				strcat_s(spell_string, spell_string_buffer, "Großes Jot, ");
				break;
			case 'K':
				strcat_s(spell_string, spell_string_buffer, "Großes Ka, ");
				break;
			case 'L':
				strcat_s(spell_string, spell_string_buffer, "Großes El, ");
				break;
			case 'M':
				strcat_s(spell_string, spell_string_buffer, "Großes Em, ");
				break;
			case 'N':
				strcat_s(spell_string, spell_string_buffer, "Großes En, ");
				break;
			case 'O':
				strcat_s(spell_string, spell_string_buffer, "Großes O, ");
				break;
			case 'P':
				strcat_s(spell_string, spell_string_buffer, "Großes Pe, ");
				break;
			case 'Q':
				strcat_s(spell_string, spell_string_buffer, "Großes Q, ");
				break;
			case 'R':
				strcat_s(spell_string, spell_string_buffer, "Großes Er, ");
				break;
			case 'S':
				strcat_s(spell_string, spell_string_buffer, "Großes Es, ");
				break;
			case 'T':
				strcat_s(spell_string, spell_string_buffer, "Großes Te, ");
				break;
			case 'U':
				strcat_s(spell_string, spell_string_buffer, "Großes U, ");
				break;
			case 'V':
				strcat_s(spell_string, spell_string_buffer, "Großes Vau, ");
				break;
			case 'W':
				strcat_s(spell_string, spell_string_buffer, "Großes We, ");
				break;
			case 'X':
				strcat_s(spell_string, spell_string_buffer, "Großes Ix, ");
				break;
			case 'Y':
				strcat_s(spell_string, spell_string_buffer, "Großes Ypsilon, ");
				break;
			case 'Z':
				strcat_s(spell_string, spell_string_buffer, "Großes Zet, ");
				break;
			case 'a':
				strcat_s(spell_string, spell_string_buffer, "Kleines A, ");
				break;
			case 'b':
				strcat_s(spell_string, spell_string_buffer, "Kleines Be, ");
				break;
			case 'c':
				strcat_s(spell_string, spell_string_buffer, "Kleines Ce, ");
				break;
			case 'd':
				strcat_s(spell_string, spell_string_buffer, "Kleines De, ");
				break;
			case 'e':
				strcat_s(spell_string, spell_string_buffer, "Kleines E, ");
				break;
			case 'f':
				strcat_s(spell_string, spell_string_buffer, "Kleines Ef, ");
				break;
			case 'g':
				strcat_s(spell_string, spell_string_buffer, "Kleines Ge, ");
				break;
			case 'h':
				strcat_s(spell_string, spell_string_buffer, "Kleines Ha, ");
				break;
			case 'i':
				strcat_s(spell_string, spell_string_buffer, "Kleines I, ");
				break;
			case 'j':
				strcat_s(spell_string, spell_string_buffer, "Kleines Jot, ");
				break;
			case 'k':
				strcat_s(spell_string, spell_string_buffer, "Kleines Ka, ");
				break;
			case 'l':
				strcat_s(spell_string, spell_string_buffer, "Kleines El, ");
				break;
			case 'm':
				strcat_s(spell_string, spell_string_buffer, "Kleines Em, ");
				break;
			case 'n':
				strcat_s(spell_string, spell_string_buffer, "Kleines En, ");
				break;
			case 'o':
				strcat_s(spell_string, spell_string_buffer, "Kleines O, ");
				break;
			case 'p':
				strcat_s(spell_string, spell_string_buffer, "Kleines Pe, ");
				break;
			case 'q':
				strcat_s(spell_string, spell_string_buffer, "Kleines Q, ");
				break;
			case 'r':
				strcat_s(spell_string, spell_string_buffer, "Kleines Er, ");
				break;
			case 's':
				strcat_s(spell_string, spell_string_buffer, "Kleines Es, ");
				break;
			case 't':
				strcat_s(spell_string, spell_string_buffer, "Kleines Te, ");
				break;
			case 'u':
				strcat_s(spell_string, spell_string_buffer, "Kleines U, ");
				break;
			case 'v':
				strcat_s(spell_string, spell_string_buffer, "Kleines Vau, ");
				break;
			case 'w':
				strcat_s(spell_string, spell_string_buffer, "Kleines We, ");
				break;
			case 'x':
				strcat_s(spell_string, spell_string_buffer, "Kleines Ix, ");
				break;
			case 'y':
				strcat_s(spell_string, spell_string_buffer, "Kleines Ypsilon, ");
				break;
			case 'z':
				strcat_s(spell_string, spell_string_buffer, "Kleines Zet, ");
				break;
			case '0':
				strcat_s(spell_string, spell_string_buffer, "Null, ");
				break;
			case '1':
				strcat_s(spell_string, spell_string_buffer, "Eins, ");
				break;
			case '2':
				strcat_s(spell_string, spell_string_buffer, "Zwei, ");
				break;
			case '3':
				strcat_s(spell_string, spell_string_buffer, "Drei, ");
				break;
			case '4':
				strcat_s(spell_string, spell_string_buffer, "Vier, ");
				break;
			case '5':
				strcat_s(spell_string, spell_string_buffer, "Fünf, ");
				break;
			case '6':
				strcat_s(spell_string, spell_string_buffer, "Sechs, ");
				break;
			case '7':
				strcat_s(spell_string, spell_string_buffer, "Sieben, ");
				break;
			case '8':
				strcat_s(spell_string, spell_string_buffer, "Acht, ");
				break;
			case '9':
				strcat_s(spell_string, spell_string_buffer, "Neun, ");
				break;
			default:
				strcat_s(spell_string, spell_string_buffer, "Unbekantes Zeichen, ");
		}
	}
	rpsay_string(spell_string);
}

void rpsay_char(char character){
	char text[]="_";
	text[0] = character;
	rpsay_spell(text);
}

void rpsay_quit(){
	fclose(espeek_pipe_handle);
	kill (espeak_pid, SIGTERM);
}