#include "rp_signalling.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>

static FILE* rps_output_pipe;
static pthread_t rps_ring_thread;

#ifdef __USE_WIRINGPI__
#include <wiringPi.h>

static volatile int rps_isrFired = 0;
static pthread_mutex_t rps_isrFired_mutex;
static volatile int rps_impulseCount = 0;
static pthread_mutex_t rps_impulseCount_mutex;

static int rps_gpio_port_in;
static int rps_gpio_port_out;

void *rps_processDial(void *arg)
{
	int i=*((int *) arg);
    free(arg);
	
	usleep(200000);

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	pthread_mutex_lock(&rps_impulseCount_mutex);
	if(i == rps_impulseCount){
		rps_impulseCount = 0;
		fputc('0'+(i==10?0:i), rps_output_pipe);
		fflush(rps_output_pipe);
	}
	pthread_mutex_unlock(&rps_impulseCount_mutex);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	fflush(stdout);
}

void *rps_processImpulse(void *arg)
{
	pthread_mutex_lock(&rps_isrFired_mutex);
        if(!rps_isrFired){
                rps_isrFired = 1;
		pthread_mutex_unlock(&rps_isrFired_mutex);
		usleep(30000);
		if(digitalRead(rps_gpio_port_in)){
			usleep(50000);
			if(!digitalRead(rps_gpio_port_in)){
				pthread_mutex_lock(&rps_impulseCount_mutex);
				rps_impulseCount++; //Valid Impulse
				if(rps_impulseCount==1){
					
					fputc('D', rps_output_pipe); //Dialing detected
					fflush(rps_output_pipe);
				}
				pthread_mutex_unlock(&rps_impulseCount_mutex);
				pthread_t thread1;
				int *arg = malloc(sizeof(*arg));
				*arg = rps_impulseCount;
				pthread_create(&thread1, NULL, rps_processDial, arg);
				pthread_detach(thread1);
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


void rps_init(int gpio_port_in, int gpio_port_out, int output_pipe) {

	rps_gpio_port_in = gpio_port_in;
	rps_gpio_port_out = gpio_port_out;
	
	rps_output_pipe = fdopen(output_pipe, "w");
	if(rps_output_pipe == NULL){
		fprintf(stderr, "rps_init: Error open output_pipe");
	}

	wiringPiSetupSys();
	wiringPiISR(rps_gpio_port_in, INT_EDGE_SETUP, rps_processEdge);

}

void *rps_processRing(void *arg){
	while(1){
		digitalWrite(rps_gpio_port_out, 1);
		sleep(2);
		digitalWrite(rps_gpio_port_out, 0);
		sleep(2);
	}
}

void rps_ring(int ring_tone){

	if(ring_tone != 0){
		pthread_create(&rps_ring_thread, NULL, rps_processRing, NULL);
	}else{
		pthread_cancel(rps_ring_thread);
		digitalWrite(rps_gpio_port_out, 0);
		rps_processEdge();
	}
	
}


void rps_quit() {
	fclose(rps_output_pipe);
}


#endif

#ifndef __USE_WIRINGPI__

static pthread_t thread1;

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

void rps_init(int gpio_port_in, int gpio_port_out, int output_pipe) {

	rps_output_pipe = fdopen(output_pipe, "w");
	if(rps_output_pipe == NULL){
		fprintf(stderr, "rps_init: Error open output_pipe\n");
	}

	pthread_create(&thread1, NULL, rps_readFromStdin, NULL);
	pthread_detach(thread1);
}

void *rps_processRing(void *arg){
	while(1){
		printf("Telefon klingelt!\n");
		fflush(stdout);
		sleep(2);
	}
}

void rps_ring(int ring_tone){

	if(ring_tone != 0){
		pthread_create(&rps_ring_thread, NULL, rps_processRing, NULL);
	}else{
		pthread_cancel(rps_ring_thread);
	}
	
}

void rps_quit() {
	pthread_cancel(thread1);
	fclose(rps_output_pipe);
}

#endif


