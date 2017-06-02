#include "rp_number2char.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
 
static int rpn2c_current_number = 0;
static int rpn2c_current_count = 0;
pthread_mutex_t rpn2c_current_mutex;

FILE* rpn2c_outputPipeHandle;
FILE* rpn2c_inputPipeHandle;
pthread_t rpn2c_pipeReadThread = NULL;

char* keyMap[] = {
	" -+_()%=0",
	".,!?&*#/1",
	"abcABC2",
	"defDEF3",
	"ghiGHI4",
	"jklJKL5",
	"mnoMNO6",
	"pqrsPQRS7",
	"tuvTUV8",
	"wxyzWXYZ9"
};

int keyMapMod[] = { 9, 9, 7, 7, 7, 7, 7, 9, 7, 9 };

void writeCharToOutput(){
	pthread_mutex_lock(&rpn2c_current_mutex);
	int number =  rpn2c_current_number;
	int count = rpn2c_current_count;
	rpn2c_current_count = 0;
	pthread_mutex_unlock(&rpn2c_current_mutex);
	count--;
	fputc(keyMap[number][count%keyMapMod[number]], rpn2c_outputPipeHandle);
	fflush(rpn2c_outputPipeHandle);
}

void *rpn2c_processTimeout(void *arg)
{
	usleep(500000);
	writeCharToOutput();
}
 

void *rpn2c_processPipeRead(void *arg)
{
	pthread_t processNumber_thread;
	
	while(feof(rpn2c_inputPipeHandle)==0){
		char readedChar = fgetc(rpn2c_inputPipeHandle);
		int readedNumber = readedChar-'0';
		
		pthread_cancel(processNumber_thread);

		if(readedChar != 'D'){
			pthread_create(&processNumber_thread, NULL, rpn2c_processTimeout, NULL);
			
			pthread_mutex_lock(&rpn2c_current_mutex);
			if(rpn2c_current_count != 0){
				if(rpn2c_current_number == readedNumber){
					rpn2c_current_count++;
				}else{
					pthread_mutex_unlock(&rpn2c_current_mutex);
					writeCharToOutput();
					pthread_mutex_lock(&rpn2c_current_mutex);
					rpn2c_current_count = 1;
				}
			}else{
				rpn2c_current_count = 1;
			}
			rpn2c_current_number = readedNumber;
			pthread_mutex_unlock(&rpn2c_current_mutex);
		}
	}
}
 
void rpn2c_init(int inputPipe, int outputPipe){
	
	rpn2c_inputPipeHandle = fdopen(inputPipe, "r");
	rpn2c_outputPipeHandle = fdopen(outputPipe, "w");
	
	pthread_create(&rpn2c_pipeReadThread, NULL, rpn2c_processPipeRead, NULL);

}

void rpn2c_quit(){
	pthread_cancel(rpn2c_pipeReadThread);
	fclose(rpn2c_inputPipeHandle);
	fclose(rpn2c_outputPipeHandle);
}