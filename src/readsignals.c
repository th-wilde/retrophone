#include <stdio.h>
#include <wiringPi.h>
#include <pthread.h>

static volatile int isrFired = 0;
pthread_mutex_t isrFired_mutex;
static volatile int impulseCount = 0;
pthread_mutex_t impulseCount_mutex;

void *processDial(void *arg)
{
	int i=0;
	pthread_mutex_lock(&impulseCount_mutex);
	i = impulseCount;
	pthread_mutex_unlock(&impulseCount_mutex);

	usleep(200000);

	pthread_mutex_lock(&impulseCount_mutex);
	if(i == impulseCount){
		impulseCount = 0;
		pthread_mutex_unlock(&impulseCount_mutex);
		printf("%d",i==10?0:i);
		fflush(stdout);
	}else{
		pthread_mutex_unlock(&impulseCount_mutex);
	}
}

void *processImpulse(void *arg)
{
	pthread_mutex_lock(&isrFired_mutex);
        if(!isrFired){
                isrFired = 1;
		pthread_mutex_unlock(&isrFired_mutex);
		usleep(30000);
		if(digitalRead(17)){
			usleep(50000);
			if(!digitalRead(17)){
				pthread_mutex_lock(&impulseCount_mutex);
				impulseCount++; //Valid Impulse
				pthread_mutex_unlock(&impulseCount_mutex);
				pthread_t thread1;
				pthread_create(&thread1, NULL, processDial, NULL);
				pthread_detach(thread1);
			}else{
				printf("H"); //Hang Up detected
				fflush(stdout);
			}
		}else{
			usleep(50000);
			printf("P"); //Pick up detected
                        fflush(stdout);
		}
		pthread_mutex_lock(&isrFired_mutex);
                isrFired = 0;
        }
	pthread_mutex_unlock(&isrFired_mutex);
}

void edge(void)
{
	pthread_t thread1;
	int test = pthread_create(&thread1, NULL, processImpulse, NULL);
	pthread_detach(thread1);
	if(test){
		printf("Error:%d;",test);
	}
}


int main() {

	//wiringPiSetupSys();
	wiringPiSetupGpio();

	wiringPiISR(17, INT_EDGE_BOTH, &edge);
	//  wiringPiISR(18, INT_EDGE_SETUP, &risingEdge);
	while(1){
		sleep(600);
/*		sleep(1);
		if(!isrFired){
			if(digitalRead(17)){
				printf("H");
			}else{
				printf("P");
			}
			fflush(stdout);
		}
*/	}
	return(0);
}

