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

static int outputPipe[2];
static int inputPipe[2];

void *processPipeRead(void *arg)
{
	FILE* test = fdopen(outputPipe[0], "r");
	while(feof(test)==0){
		rpsay_char(fgetc(test));
	}
	printf("test123\n");
	fflush(stdout);
	fclose(test);
}

int main(){
	
	rpvoip_init();
	rpsay_init();
	
	pipe(inputPipe);
	pipe(outputPipe);
	

	rps_init(GPIO_INPUT, GPIO_OUTPUT, inputPipe[1]);
	//rpn2c_init(inputPipe[0], outputPipe[1]);
	
	
	pthread_t thread1;
	pthread_create(&thread1, NULL, processPipeRead, NULL);
	
	/*FILE* test = fdopen(outputPipe[0], "r");
	while(feof(test)==0){
		
		rpsay_char(fgetc(test));
	}*/
	
	
	rpsay_spell("(ABCxyz0123456789)");
	
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
	
	rpconf_setup(config);
	
	//rpvoip_dial("015256178341");
	sleep(60);
	rpvoip_terminate();
	
	rpn2c_quit();
	rps_quit();
	rpsay_quit();
	rpvoip_quit();
	
	
	pthread_join(thread1, NULL);
	return(0);
}
