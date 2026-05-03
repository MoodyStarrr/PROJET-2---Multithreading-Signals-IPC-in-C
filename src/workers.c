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

	int run = 1;
	while( shared->STOP != 1 && run == 1){

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
		int size = snprintf(NULL,0,"data=%d\ttimestamp=%s",shared->data,time_buffer);
		char * buffer = (char * ) malloc( (size+1) * sizeof(char));
		int check = sprintf(buffer,"data=%d\ttimestamp=%s",shared->data,time_buffer);
		if( check != size ){
			printf("Couldn't write properly\n");
		}

		// "Ecriture" du message
		Message to_send;
		to_send.ptr = buffer;
		to_send.length = size;

		// Ecriture dans le pipe
		ipc_status_t status = write_msg( shared->pipe[1], &to_send);
		switch (status)
		{
			case PIPE_OK :
				run = 1;
				break;
			case PIPE_CLOSED :
				run = 0;
				printf("\nPIPE CLOSE\n");
				close( shared->pipe[1]);
				break;
			case PIPE_ERROR :
				printf("\nPIPE ERROR\n");
				run = 0;
				close( shared->pipe[1]);
				break;
			default:
				printf("\nOUT OF RANGE\n");
				run = 0;
		}
				

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

	int run = 1;
	while( shared->STOP != 1 && run == 1){

		Message received;
		// Lecture du pipe
		ipc_status_t status = read_msg( shared->pipe[0], &received);
		char * buffer = (char *) received.ptr;
		char to_log[received.length];
		switch (status)
		{
			case PIPE_OK:
				run = 1;
		
				for(int i = 0 ; i < received.length ; i++){
					//to_log[i] = buffer[i];
					printf("%c\n",buffer[i]);
				}
		
				printf("%s/n",to_log);

				// Ecriture dans le log
				fputs(to_log,shared->file);

				// Free pour éviter les leaks
				free(received.ptr);

				break;
			case PIPE_EOF:
				run = 0;
				close( shared->pipe[0] );
				break;
			case PIPE_ERROR:
				// log erreur dans le futur
				run = 0;
				break;
			default:
				printf("OUT OF RANGE\n");
				run = 0;

		}

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
