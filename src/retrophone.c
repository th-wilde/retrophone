#include "rp_signalling.h"
#include "rp_number2char.h"
#include "rp_speak.h"
#include "rp_voip.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

static int outputPipe[2];

void *processPipeRead(void *arg)
{
	FILE* test = fdopen(outputPipe[0], "r");
	while(!feof(test)){
		rpsay_char(fgetc(test));
	}

	fclose(test);
}

int main(){
	
	//rpvoip_init();
	rpsay_init();
	
	
	int inputPipe[2];
	pipe(inputPipe);
	pipe(outputPipe);
	//printf("Test\n");
	//fflush(stdout);
	rps_init(17, inputPipe[1]);
	
	rpn2c_init(inputPipe[0], outputPipe[1]);
	
	
	pthread_t thread1;
	pthread_create(&thread1, NULL, processPipeRead, NULL);
	
	/*FILE* test = fdopen(outputPipe[0], "r");
	while(!feof(test)){
		
		rpsay_char(fgetc(test));
	}*/
	
	
	rpsay_string("Wähle 015256178341");
	//rpvoip_dial("015256178341");
	sleep(10);
	//rpvoip_terminate();
	
	rpn2c_quit();
	rps_quit();
	rpsay_quit();
	//rpvoip_quit();
	
	pthread_join(thread1, NULL);
	printf("bbb\n");
	fflush(stdout);
	return(0);
}
