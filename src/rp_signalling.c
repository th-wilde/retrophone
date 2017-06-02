#include "rp_signalling.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

static FILE* rps_output_pipe;

#ifdef __USE_WIRINGPI__
#include <wiringPi.h>

static volatile int rps_isrFired = 0;
pthread_mutex_t rps_isrFired_mutex;
static volatile int rps_impulseCount = 0;
pthread_mutex_t rps_impulseCount_mutex;

static int rps_gpio_port;

void *rps_processDial(void *arg)
{
	int i=0;
	pthread_mutex_lock(&rps_impulseCount_mutex);
	i = rps_impulseCount;
	pthread_mutex_unlock(&rps_impulseCount_mutex);

	usleep(200000);

	pthread_mutex_lock(&rps_impulseCount_mutex);
	if(i == rps_impulseCount){
		rps_impulseCount = 0;
		pthread_mutex_unlock(&rps_impulseCount_mutex);
		fputc('0'+(i==10?0:i), rps_output_pipe);
		fflush(rps_output_pipe);
	}else{
		pthread_mutex_unlock(&rps_impulseCount_mutex);
	}
}

void *rps_processImpulse(void *arg)
{
	pthread_mutex_lock(&rps_isrFired_mutex);
        if(!rps_isrFired){
                rps_isrFired = 1;
		pthread_mutex_unlock(&rps_isrFired_mutex);
		usleep(30000);
		if(digitalRead(rps_gpio_port)){
			usleep(50000);
			if(!digitalRead(rps_gpio_port)){
				pthread_mutex_lock(&rps_impulseCount_mutex);
				impulseCount++; //Valid Impulse
				pthread_mutex_unlock(&rps_impulseCount_mutex);
				pthread_t thread1;
				pthread_create(&thread1, NULL, rps_processDial, NULL);
				pthread_detach(thread1);
				pthread_mutex_lock(&rps_impulseCount_mutex);
				if(impulseCount==1){
					pthread_mutex_unlock(&rps_impulseCount_mutex);
					fputc('D', rps_output_pipe); //Dialing detected
					fflush(rps_output_pipe);
				}
				pthread_mutex_unlock(&rps_impulseCount_mutex);
			}else{
				fputc('H', rps_output_pipe); //Hang Up detected
				fflush(rps_output_pipe);
			}
		}else{
			usleep(50000);
			fputc('P', rps_output_pipe); //Pick up detected
            fflush(rps_output_pipe);
		}
		pthread_mutex_lock(&rps_isrFired_mutex);
                rps_isrFired = 0;
        }
	pthread_mutex_unlock(&rps_isrFired_mutex);
}

void rps_processEdge(void)
{
	pthread_t thread1;
	pthread_create(&thread1, NULL, rps_processImpulse, NULL);
	pthread_detach(thread1);
}


void rps_init(int gpio_port, int output_pipe) {

	rps_gpio_port = gpio_port;
	rps_output_pipe = fdopen(output_pipe, "w");
	if(rps_output_pipe == null){
		fprintf(stderr, "rps_init: Error open output_pipe");
	}

	wiringPiISR(rps_gpio_port, INT_EDGE_BOTH, rps_processEdge);

	//wiringPiSetupSys();
	//wiringPiSetupGpio();

}

void rps_quit() {
	fclose(rps_output_pipe);
}


#endif

#ifndef __USE_WIRINGPI__

pthread_t thread1;

void *rps_readFromStdin(void *arg)
{
	if(rps_output_pipe == NULL){
                fprintf(stderr, "rps_init: Error open output_pipe\n");
        }
        char ch;
        while(feof(stdin)==0)
        {
                ch = fgetc(stdin);
                if((ch >= '0' && ch <= '9') || ch == 'P' || ch == 'H' || ch == 'D'){
                        fputc(ch, rps_output_pipe);
                        fflush(rps_output_pipe);
                }
        }
}

void rps_init(int gpio_port, int output_pipe){

	rps_output_pipe = fdopen(output_pipe, "w");
	if(rps_output_pipe == NULL){
                fprintf(stderr, "rps_init: Error open output_pipe\n");
        }

	pthread_create(&thread1, NULL, rps_readFromStdin, NULL);
	pthread_detach(thread1);
}

void rps_ring(int ring_tone){
	switch(ring_tone){
		default:
			
		break;
	}
}

void rps_quit() {
	pthread_cancel(thread1);
	fclose(rps_output_pipe);
}

#endif


