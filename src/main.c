#include "signal_handling.h"
#include "workers.h"

#define _POSIX_C_SOURCE_ 200809L

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

typedef struct mutex_data{
	int data;
	pthread_mutex_t mutex;
}mutex_data;

#define NB_ADD 10
#define NB_SHOW 1
#define NB_LOG 1

pthread_t threads[NB_ADD + NB_SHOW + NB_LOG];

int main(void){

	mutex_data shared_data;
	shared_data.data = 0;
	pthread_mutex_init(&shared_data.mutex,NULL);
	
	// Signal Handling
	init_signal();

	// Pthread Creation
	for(int i = 0 ; i < NB_ADD ; i++){
		if( pthread_create(&threads[i],NULL,*worker_add,&shared_data) != 0 ){
			printf("Couldn't create all adders\n");
			exit(EXIT_FAILURE);
		}
	}

	for(int i = NB_ADD ; i < NB_SHOW + NB_ADD ; i++){
		if( pthread_create(&threads[i],NULL,*worker_show,&shared_data) != 0 ){
			printf("Couldn't create all showers\n");
			exit(EXIT_FAILURE);
		}
	}

	for(int i = NB_SHOW + NB_ADD ; i < NB_SHOW + NB_ADD + NB_LOG ; i++){
		if( pthread_create(&threads[i],NULL,*worker_log,&shared_data) != 0 ){
			printf("Couldn't create all loggers\n");
			exit(EXIT_FAILURE);
		}
	}

	// Pthread Join
	for(int i = 0 ; i < NB_ADD + NB_SHOW + NB_LOG; i++){
		if( pthread_join(threads[i],NULL) != 0 ){
			printf("Couldn't join all threads\n");
			exit(EXIT_FAILURE);
		}
	}
	
	// Signal Ending
	wait_for_ending();

	pthread_mutex_destroy(&shared_data.mutex);
	return EXIT_SUCCESS;
}
