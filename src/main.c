#include "signal_handling.h"
#include "workers.h"
#include "misc.h"

#define _POSIX_C_SOURCE_ 200809L

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define NB_ADD 5
#define NB_SHOW 0
#define NB_HEART 1
#define NB_LOG 5

pthread_t threads[NB_ADD + NB_SHOW + NB_LOG + NB_HEART];

int main(void){
	// Var Init
	Configuration shared;
	shared.data = shared.STOP = shared.NB_MESSAGE_REC = shared.NB_MESSAGE_ENV = 0;

	// File Init
	//int size_path_log = snprintf(NULL,0,"logs/app_exit.log");
	//shared.file_path = (char *) malloc( sizeof(char) * size_path_log );
	shared.file_path = "logs/app_exit.log";
	shared.file = fopen(shared.file_path,"a+");

	// Pipe Init
	if( pipe(shared.pipe) == -1 ){
		printf("Couldn't create pipe\n");
		exit(EXIT_FAILURE);
	}
	//printf("main : pipe read = %d write = %d\n",shared.pipe[0],shared.pipe[1]);

	// Pthread Init
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

	for(int i = NB_SHOW + NB_ADD; i < NB_SHOW + NB_ADD + NB_HEART ; i++){
		if( pthread_create(&threads[i],NULL,*worker_heartbeat,&shared) != 0 ){
			printf("Couldn't create all loggers\n");
			exit(EXIT_FAILURE);
		}
	}

	for(int i = NB_SHOW + NB_ADD + NB_HEART ; i < NB_SHOW + NB_ADD + NB_HEART + NB_LOG ; i++){
		if( pthread_create(&threads[i],NULL,*worker_log,&shared) != 0 ){
			printf("Couldn't create all loggers\n");
			exit(EXIT_FAILURE);
		}
	}

	while( !check_stop_requested() ){
		sleep(1);
	};
	shared.STOP = check_stop_requested();

	// Pthread Join
	for(int i = 0 ; i < NB_ADD + NB_SHOW + NB_HEART; i++){
		if( pthread_join(threads[i],NULL) != 0 ){
			printf("Couldn't join all add and show threads\n");
			exit(EXIT_FAILURE);
		}
	}
	
	// Signal Ending
	wait_for_ending();
	close(shared.pipe[1]);

	for(int i = NB_ADD + NB_SHOW + NB_HEART; i < NB_HEART + NB_ADD + NB_SHOW + NB_LOG; i++){

		if( pthread_join(threads[i],NULL) != 0 ){
			printf("Couldn't join all log threads\n");
			exit(EXIT_FAILURE);
		}
	}

	close(shared.pipe[0]);

	pthread_mutex_destroy(&shared.MUTEX);
	//free(shared.file_path);
	fclose(shared.file);
	return EXIT_SUCCESS;
}
