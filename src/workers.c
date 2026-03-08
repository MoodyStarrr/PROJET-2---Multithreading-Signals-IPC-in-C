#include "workers.h"
#include "signal_handling.h"

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

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
	struct tm * timeinfo;
	struct timespec rec_log = {0,500000000};
	mutex_data * shared_data = (mutex_data * ) arg;
	static char buffer[100],time_buffer[20];

	FILE * file;
	file = fopen("logs/test.txt","a+");

	while( check_stop_requested() != 1 ){
		time( &rawtime ); // Attribue le temps depuis le 1er Janvier 1970 dans  la variable
		timeinfo = localtime( &rawtime ); // Transforme la durée en temps normal à notre période
		strftime(time_buffer,20,"%d/%m/%Y %H:%M:%S",timeinfo);// Formatage
		sprintf(buffer,"At %s,\t you had %d in shared_data\n",time_buffer,(*shared_data).data );
		fputs(buffer,file);

		nanosleep(&rec_log,NULL);
	}

	fclose(file);
	printf("Logger properly closed\n");
	pthread_exit(NULL);
}
