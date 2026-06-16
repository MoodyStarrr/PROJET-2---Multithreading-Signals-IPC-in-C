#include "signal_handling.h"
#include "workers.h"
#include "misc.h"

#define _POSIX_C_SOURCE_ 200809L

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

#define NB_SHOW 0
#define NB_HEART 1
#define NB_FIFO 1
#define NB_LOG 1


int main(void){
	// Var Init
	Configuration shared;

	if( parse_conf(&shared) == -1){
		printf("Couldn't parse configuration file\n");
		exit(EXIT_FAILURE);
	}

	// didn't want to lose time changing name everywhere
	int NB_ADD = shared.NB_WORKER_ADD;
	pthread_t threads[NB_ADD + NB_SHOW + NB_LOG + NB_HEART + NB_FIFO];

	// FIFO Creation
	shared.fifo_path = "logs/fifo";
	if( mkfifo(shared.fifo_path,0666) == -1 ){
		printf("Couldn't create FIFO\n");
		exit(EXIT_FAILURE);
	}

	// Pipe Init
	if( pipe(shared.pipe) == -1 ){
		printf("Couldn't create pipe\n");
		exit(EXIT_FAILURE);
	}

	// Pthread Init
	pthread_mutex_init(&shared.MUTEX,NULL);
	pthread_cond_init(&shared.data_ready,NULL);
	
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

	for(int i = NB_SHOW + NB_ADD + NB_HEART + NB_LOG; i < NB_SHOW + NB_ADD + NB_HEART + NB_LOG + NB_FIFO; i++){
		if( pthread_create(&threads[i],NULL,*worker_fifo,&shared) != 0 ){
			printf("Couldn't create fifo\n");
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
	if( pthread_cond_broadcast( &(shared.data_ready) ) ){
		printf("Failed broadcast\n");
		exit(EXIT_FAILURE);
	}

	close(shared.pipe[1]);

	for(int i = NB_ADD + NB_SHOW + NB_HEART; i < NB_HEART + NB_ADD + NB_SHOW + NB_LOG + NB_FIFO; i++){

		if( pthread_join(threads[i],NULL) != 0 ){
			printf("Couldn't join all log threads\n");
			exit(EXIT_FAILURE);
		}
	}

	close(shared.pipe[0]);

	pthread_mutex_destroy(&shared.MUTEX);
	pthread_cond_destroy(&shared.data_ready);
	free(shared.file_path);
	free(shared.format);
	fclose(shared.file);
	remove(shared.fifo_path);
	return EXIT_SUCCESS;
}
