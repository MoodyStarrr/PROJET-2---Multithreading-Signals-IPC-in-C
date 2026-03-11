#include "signal_handling.h"
#include "workers.h"
#include "misc.h"

#define _POSIX_C_SOURCE_ 200809L

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define NB_ADD 10
#define NB_SHOW 1
#define NB_LOG 1

pthread_t threads[NB_ADD + NB_SHOW + NB_LOG];

int main(void){

	Configuration shared;
	shared.data = 0;
	pthread_mutex_init(&shared.MUTEX,NULL);
	
	// Signal Handling
	init_signal();

	// Pthread Creation
	for(int i = 0 ; i < NB_ADD ; i++){
		if( pthread_create(&threads[i],NULL,*worker_add,&shared) != 0 ){
			printf("Couldn't create all adders\n");
			exit(EXIT_FAILURE);
		}
	}

	for(int i = NB_ADD ; i < NB_SHOW + NB_ADD ; i++){
		if( pthread_create(&threads[i],NULL,*worker_show,&shared) != 0 ){
			printf("Couldn't create all showers\n");
			exit(EXIT_FAILURE);
		}
	}

	for(int i = NB_SHOW + NB_ADD ; i < NB_SHOW + NB_ADD + NB_LOG ; i++){
		if( pthread_create(&threads[i],NULL,*worker_log,&shared) != 0 ){
			printf("Couldn't create all loggers\n");
			exit(EXIT_FAILURE);
		}
	}

	while( (shared.STOP = check_stop_requested()) ){
		sleep(1);
	};

	// Pthread Join
	for(int i = 0 ; i < NB_ADD + NB_SHOW + NB_LOG; i++){
		if( pthread_join(threads[i],NULL) != 0 ){
			printf("Couldn't join all threads\n");
			exit(EXIT_FAILURE);
		}
	}
	
	// Signal Ending
	wait_for_ending();

	pthread_mutex_destroy(&shared.MUTEX);
	return EXIT_SUCCESS;
}
