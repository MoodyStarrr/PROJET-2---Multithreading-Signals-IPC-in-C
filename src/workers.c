#include "workers.h"
#include "signal_handling.h"

#define _POSIX_C_SOURCE 200809L
#define TIME_LENGTH 20 

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

typedef struct mutex_data{
	int data;
	pthread_mutex_t mutex;
}mutex_data;

void * worker_add(void * arg){
	struct timespec rec_add = {0,250000000};
	mutex_data * shared_data = (mutex_data * ) arg;

	while(check_stop_requested() != 1 ){
		pthread_mutex_lock( (&shared_data->mutex) );

		(*shared_data).data++;

		pthread_mutex_unlock( (&shared_data->mutex) );
		nanosleep(&rec_add,NULL);
	}
	printf("Add properly closed\n");

	pthread_exit(NULL);
}

void * worker_show(void * arg){
	struct timespec rec_show = {1,0};
	mutex_data * shared_data = (mutex_data * ) arg;

	while(check_stop_requested() != 1 ){
		pthread_mutex_lock( (&shared_data->mutex) );

		printf("%d in shared_data increment\n",(*shared_data).data);

		pthread_mutex_unlock( (&shared_data->mutex) );
		nanosleep(&rec_show,NULL);
	}
	printf("Show properly closed\n");

	pthread_exit(NULL);
}

void * worker_log(void * arg){
	time_t rawtime;
	struct timespec rec_log = {0,500000000};
	mutex_data * shared_data = (mutex_data * ) arg;
	static char time_buffer[TIME_LENGTH];

	FILE * file = fopen("logs/test.txt","a+");

	while( check_stop_requested() != 1 ){
	       	// Attribue le temps depuis le 1er Janvier 1970 dans  la variable
		time( &rawtime );

	       	// Transforme la durée en temps normal à notre période
		struct tm * timeinfo = localtime( &rawtime );

		// Formatage
		strftime(time_buffer,TIME_LENGTH,"%d/%m/%Y %H:%M:%S",timeinfo);
		
		// Avoir la taille du message avant de malloc
		int size = snprintf(NULL,0,"[%s]\tshared_data = %d;\n",time_buffer,(*shared_data).data );

		// Var avec taille parfaite
		char * buffer = (char *) malloc( size * sizeof(char) );

		// Ecriture
		sprintf(buffer,"[%s]\t shared_data = %d;\n",time_buffer,(*shared_data).data );
		fputs(buffer,file);

		// Free pour éviter les leaks
		free(buffer);

		nanosleep(&rec_log,NULL);
	}

	fclose(file);
	printf("Logger properly closed\n");
	pthread_exit(NULL);
}
