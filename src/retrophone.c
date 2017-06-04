#include <stdio.h>
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

enum state_enum {IDLE, RING, CALL, CONFIG, DIAL, ENDED};
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
	
	
	/*enum state_enum voip_state = NORMAL;
	while(1){
		
		pthread_mutex_lock(&state_mutex);
		switch(current_state){
			case CALL:
				current_state = ENDED;
			break;
			case ENDED:
				current_state = NORMAL;
			break;
			case RING:
				if(voip_state==NORMAL){
					rps_ring(0);
					current_state = NORMAL;
				}else{
					current_state = CALL;
				}
			break;
			default:
				switch(voip_state){
					case RING:
						rps_ring(1);
						current_state = RING;
					break;
					case CALL:
						current_state = CALL;
				}
		}
		pthread_mutex_unlock(&state_mutex);
		
		voip_state = rpvoip_update();
	}*/
}

void *signalling_thread(void *arg)
{
	FILE* test = fdopen(signallingPipe[0], "r");
	while(feof(test)==0){
		fputc(fgetc(test), combinedPipeWriteHandle);
		fflush(combinedPipeWriteHandle);
		/*char input = fgetc(test);
		pthread_mutex_lock(&state_mutex);
		switch(current_state){
			case RING:
				if(input == 'P'){
					rps_ring(0);
					rpvoip_answer();
				}
			break;
			case NORMAL:
				current_state = DIAL;
			break;
			case CALL:
				switch(input){
					case 'H':
						rpvoip_terminate();	
						current_state = ENDED;
					break;
					default:
						if(input >= '0' && input <= '9'){
							rpvoip_dial(input);
						}
				}
			break;
			case ENDED:
				if(input == 'H'){
					current_state = NORMAL;
				}
		}
		pthread_mutex_unlock(&state_mutex);*/
	}
	fclose(test);
}

void *dial_timeout_thread(void *arg)
{
	sleep(4);
	fputc('T', combinedPipeWriteHandle);
	fflush(combinedPipeWriteHandle);
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
						dialed_numer[0] = 0x00;
						current_state = IDLE;
					break;
					case 'R':
						rpvoip_terminate();
					break;
					case 'T':
						rpsay_string("Wähle Telefonnummer");
						rpsay_spell(dialed_numer);
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
						if(dialTimeoutThread_handle){
							pthread_cancel(dialTimeoutThread_handle);
						}
						if(input >= '0' && input <= '9'){
							char append[2];
							append[0] = input;
							append[1] = 0x00;
							strcat_s(dialed_numer, dialed_numer_buffer, append);
							pthread_create(&dialTimeoutThread_handle, NULL, dial_timeout_thread, NULL);
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
		}
		
		fputc(input, stdout);
		fflush(stdout);
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
