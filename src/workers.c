#include "workers.h"
#include "signal_handling.h"
#include "misc.h"

#define _POSIX_C_SOURCE 200809L
#define TIME_LENGTH 20 

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

void * worker_add(void * arg){
	struct timespec rec_add = {0,250000000};

	Configuration * shared = (Configuration * ) arg;

	static char time_buffer[TIME_LENGTH];
	time_t rawtime;

	while( shared->STOP != 1 ){

	       	// Attribue le temps depuis le 1er Janvier 1970 dans  la variable
		time( &rawtime );
	       	// Transforme la durée en temps normal à notre période
		struct tm * timeinfo = localtime( &rawtime );
		// Formatage
		strftime(time_buffer,TIME_LENGTH,"%d/%m/%Y %H:%M:%S",timeinfo);

		pthread_mutex_lock( &(shared->MUTEX) );
		close(shared->pipe[0]);

		shared->data++;

		char * buffer;
		int size = snprintf(NULL,0,"buffer=%p,data=%d,timestamp=%s",buffer,shared->data,time_buffer);
		size = snprintf(NULL,0,"length=%d,buffer=%p,data=%d,timestamp=%s",size,buffer,shared->data,time_buffer);
		buffer = (char * ) malloc( size * sizeof(char));
		sprintf(buffer,"length=%d,buffer=%p,data=%d,timestamp=%s",size,buffer,shared->data,time_buffer);

		write_str(shared->pipe_id,buffer);

		close(shared->pipe([1]);
		pthread_mutex_unlock( &(shared->MUTEX) );

		nanosleep(&rec_add,NULL);
	}

//	printf("Add properly closed\n");

	pthread_exit(NULL);
}

void * worker_show(void * arg){
	struct timespec rec_show = {1,0};
	Configuration * shared = (Configuration * ) arg;

	while( shared->STOP != 1 ){
		pthread_mutex_lock( &(shared->MUTEX) );

		printf("%d in shared increment\n",shared->data);
		
		pthread_mutex_unlock( &(shared->MUTEX) );
		nanosleep(&rec_show,NULL);
	}

//	printf("Show properly closed\n");

	pthread_exit(NULL);
}

void * worker_log(void * arg){
	struct timespec rec_log = {0,500000000};
	Configuration * shared = (Configuration * ) arg;

	while( shared->STOP != 1 ){
		// Lecture du pipe


		// Déchiffrage du message recu

		// Ecriture dans le log
		// sprintf(buffer,"[%s]\t shared = %d;\n",time_buffer,shared->data);
		fputs(buffer,shared->file);

		// Free pour éviter les leaks
		free(buffer);

		nanosleep(&rec_log,NULL);
	}

	fclose(shared->file);
//	printf("Logger properly closed\n");
	pthread_exit(NULL);
}
