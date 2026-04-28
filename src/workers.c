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
	close(shared->pipe[0]);
	printf("Fermeture du bout de lecture\n");

	static char time_buffer[TIME_LENGTH];
	time_t rawtime;

	while( shared->STOP != 1 ){

	       	// Attribue le temps depuis le 1er Janvier 1970 dans  la variable
		time( &rawtime );
	       	// Transforme la durée en temps normal à notre période
		struct tm * timeinfo = localtime( &rawtime );
		// Formatage
		strftime(time_buffer,TIME_LENGTH,"%d/%m/%Y %H:%M:%S",timeinfo);

		// Race Protection
		pthread_mutex_lock( &(shared->MUTEX) );

		// Increment
		shared->data++;

		// Generation du texte
		char * buffer;
		int size = snprintf(NULL,0,"data=%d\ttimestamp=%s",shared->data,time_buffer);
		buffer = (char * ) malloc( (size + 1) * sizeof(char));

		// "Ecriture" du message
		Message to_send;
		to_send.ptr = buffer;
		to_send.length = size;

		// Ecriture dans le pipe
		write_msg(shared->pipe[1],&to_send);

		// Race Protection
		pthread_mutex_unlock( &(shared->MUTEX) );

		// Périodicicté
		nanosleep(&rec_add,NULL);
	}

	pthread_exit(NULL);
}

void * worker_log(void * arg){
	struct timespec rec_log = {0,500000000};
	Configuration * shared = (Configuration * ) arg;
	close(shared->pipe[1]);
	printf("Fermeture du bout d'écriture\n");

	while( shared->STOP != 1 ){

		Message received;
		// Lecture du pipe
		if (read(shared->pipe[0],&received,sizeof(received)) == -1){
			printf("Couldn't read\n");
			exit(EXIT_FAILURE);
		};

		// Déchiffrage du message recu
		char * buffer = (char *) received.ptr;
		char to_log[received.length + 1];
		
		/*
		for(int i = 0 ; i < received.length ; i++){
			to_log[i] = buffer[i];
		}
		to_log[received.length + 1] = '\0';
		*/

		printf("%s/n",buffer);

		// Ecriture dans le log
		fputs(to_log,shared->file);

		// Free pour éviter les leaks
		free(received.ptr);

		nanosleep(&rec_log,NULL);
	}

	fclose(shared->file);
	pthread_exit(NULL);
}

void * worker_show(void * arg){
	struct timespec rec_show = {1,0};
	Configuration * shared = (Configuration * ) arg;

	while( shared->STOP != 1 ){
		pthread_mutex_lock( &(shared->MUTEX) );

		//printf("%d in shared increment\n",shared->data);
		
		pthread_mutex_unlock( &(shared->MUTEX) );
		nanosleep(&rec_show,NULL);
	}

	pthread_exit(NULL);
}
