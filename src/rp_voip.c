#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <regex.h> 
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "popen2.h"
#include "string_s.h"
#include "rp_voip.h"


static pthread_t rpvoip_pipeReadThread;
static FILE* rpvoip_outputPipeHandle;
static FILE* rpvoip_inputPipeHandle;
static pid_t rpvoip_linphone_pid;

static enum rpvoip_state_enum rpvoip_state1 = NORMAL;
static enum rpvoip_state_enum rpvoip_state2 = NORMAL;
static pthread_mutex_t rpvoip_state_mutex1;
static pthread_mutex_t rpvoip_state_mutex2;
static int rpvoip_alternate_pointer1 = 0;
static int rpvoip_alternate_pointer2 = 0;

static regex_t rpvoip_regex_ring;
static regex_t rpvoip_regex_call;
static regex_t rpvoip_regex_normal;

void *rpvoip_processPipeRead(void *arg)
{
	int linebuffer=256;
	char line[linebuffer];
	
	
	while(feof(rpvoip_outputPipeHandle)==0){
		fgets(line, linebuffer, rpvoip_outputPipeHandle);
		enum rpvoip_state_enum state;
		if(!regexec(&rpvoip_regex_normal, line, 0, NULL, 0)){ //Call ended -> goto normal state
			state = NORMAL;
		}else if(!regexec(&rpvoip_regex_ring, line, 0, NULL, 0)){ //Call incoming -> goto ring state
			state = RING;
		}else if(!regexec(&rpvoip_regex_call, line, 0, NULL, 0)){ //Call answered -> goto call state
			state = CALL;
		}
		
		if(rpvoip_alternate_pointer1==0){
			if(state != rpvoip_state1){
				rpvoip_alternate_pointer1 = (rpvoip_alternate_pointer1 + 1) % 2;
				pthread_mutex_lock(&rpvoip_state_mutex1);
				rpvoip_state1 = state;
				pthread_mutex_unlock(&rpvoip_state_mutex1);
			}
		}else{
			if(state != rpvoip_state2){
				rpvoip_alternate_pointer1 = (rpvoip_alternate_pointer1 + 1) % 2;
				pthread_mutex_lock(&rpvoip_state_mutex2);
				rpvoip_state2 = state;
				pthread_mutex_unlock(&rpvoip_state_mutex2);
			}
		}
	}
}

void rpvoip_init(){
	
	int rpvoip_linphone_pipe[2];
	
	char* command[] = {"linphonec", NULL};
	rpvoip_linphone_pid = popen2(command, &rpvoip_linphone_pipe[1], &rpvoip_linphone_pipe[0]);
	fflush(stdout);

	
	rpvoip_inputPipeHandle = fdopen(rpvoip_linphone_pipe[1], "w");
	rpvoip_outputPipeHandle = fdopen(rpvoip_linphone_pipe[0], "r");
	
	regcomp(&rpvoip_regex_ring, "Receiving new incoming call from .*<(.*)>, assigned id ([[:digit:]]+)", 0);
	regcomp(&rpvoip_regex_call, "Call ([[:digit:]]+) with .*<(.*)> connected|Call ([[:digit:]]+) to .*<(.*)> in progress", 0);
	regcomp(&rpvoip_regex_normal, "Call ([[:digit:]]+) with .*<(.*)> (ended|error)", 0);
	fflush(stdout);
	
	pthread_create(&rpvoip_pipeReadThread, NULL, rpvoip_processPipeRead, NULL);
}
void rpvoip_dial(char* number){
	enum rpvoip_state_enum state;
	if(rpvoip_alternate_pointer1==0){
		state = rpvoip_state1;
	}else{
		state = rpvoip_state2;
	}
	if(state==NORMAL){
		int sipAddressBuffer=256;
		char sipAddress[sipAddressBuffer];
		sipAddress[0] = 0x00;
		strcat_s(sipAddress, sipAddressBuffer, "call ");
		strcat_s(sipAddress, sipAddressBuffer, number);
		strcat_s(sipAddress, sipAddressBuffer, "@sipgate.de\n");
		fputs(sipAddress, rpvoip_inputPipeHandle);
		fflush(rpvoip_inputPipeHandle);
	}else if(state==CALL){
		for(int i=0; i<strlen(number); i++){
			char numberToDial[3] = "0\n";
			numberToDial[0] = number[i];
			fputs(numberToDial, rpvoip_inputPipeHandle);
			fflush(rpvoip_inputPipeHandle);
		}
	}
}

enum rpvoip_state_enum rpvoip_update() {
	rpvoip_alternate_pointer2 = (rpvoip_alternate_pointer2 + 1) % 2;
	enum rpvoip_state_enum result;
	if(rpvoip_alternate_pointer2==0){
		pthread_mutex_lock(&rpvoip_state_mutex1);
		result = rpvoip_state1;
		pthread_mutex_unlock(&rpvoip_state_mutex1);
	}else{
		pthread_mutex_lock(&rpvoip_state_mutex2);
		result = rpvoip_state2;
		pthread_mutex_unlock(&rpvoip_state_mutex2);
	}
	
	return result;
}

void rpvoip_answer(){
	fputs("answer\n", rpvoip_inputPipeHandle);
	fflush(rpvoip_inputPipeHandle);
}

void rpvoip_terminate(){
	fputs("terminate\n", rpvoip_inputPipeHandle);
	fflush(rpvoip_inputPipeHandle);
}

void rpvoip_quit() {
	
	fputs("quit\n", rpvoip_inputPipeHandle);
	fflush(rpvoip_inputPipeHandle);
	
	waitpid(rpvoip_linphone_pid, NULL, WUNTRACED);
	
	pthread_cancel(rpvoip_pipeReadThread);
	
	/*
	regfree(&rpvoip_regex_ring);
	regfree(&rpvoip_regex_call);
	regfree(&rpvoip_regex_normal);
	
	fclose(rpvoip_inputPipeHandle);
	fclose(rpvoip_outputPipeHandle);*/
}
