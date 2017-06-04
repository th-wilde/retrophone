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
#include "rp_config_struct.h"


static pthread_t rpvoip_pipeReadThread;
static FILE* rpvoip_outputLinphoneHandle;
static FILE* rpvoip_inputLinphoneHandle;
static pid_t rpvoip_linphone_pid;
static FILE* rpvoip_output_pipe;
static int rpvoip_sip_realm_buffer = 256;
static char rpvoip_sip_realm[256];

static regex_t rpvoip_regex_ring;
static regex_t rpvoip_regex_call;
static regex_t rpvoip_regex_normal;

void *rpvoip_processPipeRead(void *arg)
{
	int linebuffer=256;
	char line[linebuffer];
	
	while(feof(rpvoip_outputLinphoneHandle)==0){
		fgets(line, linebuffer, rpvoip_outputLinphoneHandle);
		if(!regexec(&rpvoip_regex_normal, line, 0, NULL, 0)){ //Call ended -> goto normal state
			fputc('N', rpvoip_output_pipe);
			fflush(rpvoip_output_pipe);
		}else if(!regexec(&rpvoip_regex_ring, line, 0, NULL, 0)){ //Call incoming -> goto ring state
			fputc('R', rpvoip_output_pipe);
			fflush(rpvoip_output_pipe);
		}else if(!regexec(&rpvoip_regex_call, line, 0, NULL, 0)){ //Call answered -> goto call state
			fputc('C', rpvoip_output_pipe);
			fflush(rpvoip_output_pipe);
		}
		/*printf(line);
		fflush(stdout);*/
	}
}

void rpvoip_init(struct rp_config_struct config, int output_pipe){
	
	strcpy_s(rpvoip_sip_realm, rpvoip_sip_realm_buffer, config.sip_realm);
	
	rpvoip_output_pipe = fdopen(output_pipe, "w");
	if(rpvoip_output_pipe == NULL){
		fprintf(stderr, "rpvoip_init: Error open output_pipe");
	}
	
	int rpvoip_linphone_pipe[2];
	
	char* command[] = {"linphonec", NULL};
	rpvoip_linphone_pid = popen2(command, &rpvoip_linphone_pipe[1], &rpvoip_linphone_pipe[0]);
	fflush(stdout);

	
	rpvoip_inputLinphoneHandle = fdopen(rpvoip_linphone_pipe[1], "w");
	rpvoip_outputLinphoneHandle = fdopen(rpvoip_linphone_pipe[0], "r");
	
	regcomp(&rpvoip_regex_ring, "Receiving new incoming call from .*<(.*)>, assigned id ([[:digit:]]+)", REG_EXTENDED);
	regcomp(&rpvoip_regex_call, "Call ([[:digit:]]+) with .*<(.*)> connected|Call ([[:digit:]]+) to .*<(.*)> in progress", REG_EXTENDED);
	regcomp(&rpvoip_regex_normal, "Call ([[:digit:]]+) with .*<(.*)> (ended|error)", REG_EXTENDED);
	fflush(stdout);
	
	pthread_create(&rpvoip_pipeReadThread, NULL, rpvoip_processPipeRead, NULL);
}

void rpvoip_call(char* number){
	int sipAddressBuffer=256;
	char sipAddress[sipAddressBuffer];
	sipAddress[0] = 0x00;
	strcat_s(sipAddress, sipAddressBuffer, "call ");
	strcat_s(sipAddress, sipAddressBuffer, number);
	strcat_s(sipAddress, sipAddressBuffer, "@");
	strcat_s(sipAddress, sipAddressBuffer, rpvoip_sip_realm);
	strcat_s(sipAddress, sipAddressBuffer, "\n");
	fputs(sipAddress, rpvoip_inputLinphoneHandle);
	fflush(rpvoip_inputLinphoneHandle);
}

void rpvoip_dial(char number){
	char numberToDial[3] = "0\n";
	numberToDial[0] = number;
	fputs(numberToDial, rpvoip_inputLinphoneHandle);
	fflush(rpvoip_inputLinphoneHandle);
}

void rpvoip_answer(){
	fputs("answer\n", rpvoip_inputLinphoneHandle);
	fflush(rpvoip_inputLinphoneHandle);
}

void rpvoip_terminate(){
	fputs("terminate\n", rpvoip_inputLinphoneHandle);
	fflush(rpvoip_inputLinphoneHandle);
}

void rpvoip_quit() {
	
	fputs("quit\n", rpvoip_inputLinphoneHandle);
	fflush(rpvoip_inputLinphoneHandle);
	
	waitpid(rpvoip_linphone_pid, NULL, WUNTRACED);
	
	pthread_cancel(rpvoip_pipeReadThread);
	
	fclose(rpvoip_output_pipe);
	
	/*
	regfree(&rpvoip_regex_ring);
	regfree(&rpvoip_regex_call);
	regfree(&rpvoip_regex_normal);
	
	fclose(rpvoip_inputLinphoneHandle);
	fclose(rpvoip_outputLinphoneHandle);*/
}
