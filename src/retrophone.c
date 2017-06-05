#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "rp_signalling.h"
#include "rp_number2char.h"
#include "rp_speak.h"
#include "rp_voip.h"
#include "rp_config.h"
#include "string_s.h"

#define GPIO_INPUT 17
#define GPIO_OUTPUT 27

static int combinedPipe[2];
static int signallingPipe[2];
static int voipPipe[2];
static FILE* combinedPipeWriteHandle;

static pthread_mutex_t dialTimeoutIndicator_mutex;
int dialTimeoutIndicator = -1;

enum state_enum {IDLE, RING, CALL, CONFIG, DIAL};
//static pthread_mutex_t state_mutex;
//enum state_enum current_state = NORMAL;

void *voip_thread(void *arg)
{
	FILE* test = fdopen(voipPipe[0], "r");
	while(feof(test)==0){
		fputc(fgetc(test), combinedPipeWriteHandle);
		fflush(combinedPipeWriteHandle);
	}
	fclose(test);
}

void *signalling_thread(void *arg)
{
	FILE* test = fdopen(signallingPipe[0], "r");
	while(feof(test)==0){
		fputc(fgetc(test), combinedPipeWriteHandle);
		fflush(combinedPipeWriteHandle);
	}
	fclose(test);
}

struct timeout_arg_struct {
	int timeout;
	char signal;
};

void *dial_timeout_thread(void *arg)
{
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	struct timeout_arg_struct arg2=*((struct timeout_arg_struct *) arg);
    free(arg);
	if(arg2.timeout < 0)
		pthread_exit(NULL);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	
	sleep(arg2.timeout);
	
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	fputc(arg2.signal, combinedPipeWriteHandle);
	fflush(combinedPipeWriteHandle);
	pthread_mutex_lock(&dialTimeoutIndicator_mutex);
	dialTimeoutIndicator=1;
	pthread_mutex_unlock(&dialTimeoutIndicator_mutex);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
}

int main(){
	
	struct rp_config_struct config;
	rpconf_read(config);
	
	strcpy_s(config.sip_server, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, "sipgate.de");
	strcpy_s(config.sip_username, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, "2497424e0");
	strcpy_s(config.sip_password, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, "sU6qVXGU3YjH");
	strcpy_s(config.sip_realm, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, "sipgate.de");
	strcpy_s(config.stun_server, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, "stun.sipgate.net");
	strcpy_s(config.wifi_ssid, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, "wilnetwlan");
	strcpy_s(config.wifi_password, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, "aaaaaaa");
	
	rpconf_write(config);
	
	
	pipe(signallingPipe);
	pipe(voipPipe);
	pipe(combinedPipe);
	
	combinedPipeWriteHandle = fdopen(combinedPipe[1], "w");
	if(combinedPipeWriteHandle == NULL){
		fprintf(stderr, "retrophone: Error open output_pipe");
	}
	
	rpvoip_init(config, voipPipe[1]);
	rpsay_init();
	rps_init(GPIO_INPUT, GPIO_OUTPUT, signallingPipe[1]);
	//rpn2c_init(signallingPipe[0], combinedPipe[1]);
	
	
	pthread_t voipThread_handle;
	pthread_t signallingThread_handle;
	pthread_create(&voipThread_handle, NULL, voip_thread, NULL);
	pthread_create(&signallingThread_handle, NULL, signalling_thread, NULL);
	
	
	pthread_t dialTimeoutThread_handle = NULL;
	
	enum state_enum current_state = IDLE;
	int dialed_numer_buffer=256;
	char dialed_numer[dialed_numer_buffer];
	dialed_numer[0] = 0x00;
	
	FILE* test = fdopen(combinedPipe[0], "r");
	while(feof(test)==0){
		char input = fgetc(test);
		fputc(input, stdout);
		fflush(stdout);
		switch(current_state){
			case IDLE:
				switch(input){
					case 'R':
						rps_ring(1);
						current_state = RING;
					break;
					case 'P':
						current_state = DIAL;
					break;
					case 'C':
						rpvoip_terminate();
					break;
				}
			break;
			case RING:
				switch(input){
					case 'P':
						rps_ring(0);
						rpvoip_answer();
					break;
					case 'N':
						rps_ring(0);
						current_state = IDLE;
					break;
					case 'C':
						rps_ring(0);
						current_state = CALL;
					break;
				}
			break;
			case DIAL:
				switch(input){
					case 'H':
						pthread_mutex_lock(&dialTimeoutIndicator_mutex);
						if(dialTimeoutIndicator==0){
							pthread_cancel(dialTimeoutThread_handle);
							dialTimeoutIndicator=0;
						}
						pthread_mutex_unlock(&dialTimeoutIndicator_mutex);
						dialed_numer[0] = 0x00;
						current_state = IDLE;
					break;
					case 'R':
						rpvoip_terminate();
					break;
					case 'T':
						if(strcmp(dialed_numer,"0000") == 0){
							dialed_numer[0] = 0x00;
							current_state = CONFIG;
						}
						rpsay_string("Waehle Telephonenummer");
						rpsay_spell(dialed_numer);
						if(dialTimeoutIndicator==0){
							pthread_cancel(dialTimeoutThread_handle);
						}
						pthread_mutex_unlock(&dialTimeoutIndicator_mutex);
						struct timeout_arg_struct *arg = malloc(sizeof(*arg));
						(*arg).timeout = 4;
						(*arg).signal = 'S';
						pthread_create(&dialTimeoutThread_handle, NULL, dial_timeout_thread, arg);
					break;
					case 'S':
						rpvoip_call(dialed_numer);
					break;
					case 'C':
						dialed_numer[0] = 0x00;
						current_state = CALL;
					break;
					case 'D':
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						pthread_mutex_lock(&dialTimeoutIndicator_mutex);
						if(dialTimeoutIndicator==0){
							pthread_cancel(dialTimeoutThread_handle);
						}
						pthread_mutex_unlock(&dialTimeoutIndicator_mutex);
						if(input >= '0' && input <= '9'){
							char append[2];
							append[0] = input;
							append[1] = 0x00;
							strcat_s(dialed_numer, dialed_numer_buffer, append);
							struct timeout_arg_struct *arg = malloc(sizeof(*arg));
							(*arg).timeout = 4;
							(*arg).signal = 'T';
							pthread_create(&dialTimeoutThread_handle, NULL, dial_timeout_thread, arg);
							pthread_mutex_lock(&dialTimeoutIndicator_mutex);
							dialTimeoutIndicator=0;
							pthread_mutex_unlock(&dialTimeoutIndicator_mutex);
							pthread_detach(dialTimeoutThread_handle);
						}
					break;
				}
			break;
			case CALL:
				switch(input){
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						rpvoip_dial(input);
					break;
					case 'H':
						rpvoip_terminate();
					case 'N':
						current_state = IDLE;
					break;
				}
			break;
			case CONFIG:
				switch(input){
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						rpvoip_dial(input);
					break;
					case 'H':
						rpvoip_terminate();
					case 'N':
						current_state = IDLE;
					break;
				}
			break;
		}
	}
	fclose(test);
	
	/*rpsay_spell("(ABCxyz0123456789)");
	
	
	
	strcpy_s(config.sip_server, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, "sipgate.de");
	strcpy_s(config.sip_username, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, "2497424e0");
	strcpy_s(config.sip_password, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, "sU6qVXGU3YjH");
	strcpy_s(config.sip_realm, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, "sipgate.de");
	strcpy_s(config.stun_server, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, "stun.sipgate.net");
	strcpy_s(config.wifi_ssid, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, "wilnetwlan");
	strcpy_s(config.wifi_password, RP_CONFIG_STRUCT_STRING_BUFFER_DEFINE, "aaaaaaa");
	
	rpconf_write(config);
	
	rpconf_setup(config);
	
	//rpvoip_dial("015256178341");
	sleep(60);
	rpvoip_terminate();*/
	
	//rpn2c_quit();
	rps_quit();
	rpsay_quit();
	rpvoip_quit();
	
	pthread_join(signallingThread_handle, NULL);
	pthread_join(voipThread_handle, NULL);
	return(0);
}
