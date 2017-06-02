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
#include "rp_voip.h"


static pthread_t rpvoip_pipeReadThread;
static FILE* rpvoip_outputPipeHandle;
static FILE* rpvoip_inputPipeHandle;
static pid_t rpvoip_linphone_pid;

static enum rpvoip_state_enum rpvoip_state = NORMAL;
static enum rpvoip_state_enum rpvoip_old_state = NORMAL;
static pthread_mutex_t rpvoip_state_mutex;

static regex_t rpvoip_regex_ring;
static regex_t rpvoip_regex_call;
static regex_t rpvoip_regex_normal;


static void strcat_s(char* str, int length, char *cat){
	if(strlen(str) + strlen(cat) + 1 < length){
		strcat(str, cat);
	}else{
		fprintf(stderr, "error concat oveflow: %s %s > %d", str, cat, length);
	}
}

void *rpvoip_processPipeRead(void *arg)
{
	int linebuffer=256;
	char line[linebuffer];
	
	
	while(feof(rpvoip_outputPipeHandle)==0){
		fgets(line, linebuffer, rpvoip_outputPipeHandle);
		if(!regexec(&rpvoip_regex_normal, "abc", 0, NULL, 0)){ //Call ended -> goto normal state
			pthread_mutex_lock(&rpvoip_state_mutex);
			rpvoip_state = NORMAL;
			pthread_mutex_unlock(&rpvoip_state_mutex);
		}else if(!regexec(&rpvoip_regex_ring, "abc", 0, NULL, 0)){ //Call incoming -> goto ring state
			pthread_mutex_lock(&rpvoip_state_mutex);
			rpvoip_state = RING;
			pthread_mutex_unlock(&rpvoip_state_mutex);
		}else if(!regexec(&rpvoip_regex_call, "abc", 0, NULL, 0)){ //Call answered -> goto call state
			pthread_mutex_lock(&rpvoip_state_mutex);
			rpvoip_state = CALL;
			pthread_mutex_unlock(&rpvoip_state_mutex);
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
	pthread_mutex_lock(&rpvoip_state_mutex);
	if(rpvoip_state==NORMAL){
		pthread_mutex_unlock(&rpvoip_state_mutex);
		int sipAddressBuffer=256;
		char sipAddress[sipAddressBuffer];
		sipAddress[0] = 0x00;
		strcat_s(sipAddress, sipAddressBuffer, "call ");
		strcat_s(sipAddress, sipAddressBuffer, number);
		strcat_s(sipAddress, sipAddressBuffer, "@sipgate.de \n");
		fputs(sipAddress, rpvoip_inputPipeHandle);
		fflush(rpvoip_inputPipeHandle);
	}else if(rpvoip_state==CALL){
		pthread_mutex_unlock(&rpvoip_state_mutex);
		for(int i=0; i<strlen(number); i++){
			char numberToDial[3] = "0\n";
			numberToDial[0] = number[i];
			fputs(numberToDial, rpvoip_inputPipeHandle);
			fflush(rpvoip_inputPipeHandle);
		}
	}
	pthread_mutex_unlock(&rpvoip_state_mutex);
}

bool rpvoip_update() {
	pthread_mutex_lock(&rpvoip_state_mutex);
	if(rpvoip_old_state != rpvoip_state){
		rpvoip_old_state = rpvoip_state;
		pthread_mutex_unlock(&rpvoip_state_mutex);
		return true;
	}
	pthread_mutex_unlock(&rpvoip_state_mutex);
	return false;
}
enum rpvoip_state_enum rpvoip_get_state(){
	pthread_mutex_lock(&rpvoip_state_mutex);
	enum rpvoip_state_enum result = rpvoip_state;
	pthread_mutex_unlock(&rpvoip_state_mutex);
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
