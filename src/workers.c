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
#include <signal.h>
#include <math.h>
#include <poll.h>
#include <fcntl.h>

void * worker_add(void * arg){
	struct timespec rec_add = {0,250000000};

	Configuration * shared = (Configuration * ) arg;

	static char time_buffer[TIME_LENGTH];
	time_t rawtime;

	int run = 1;
	while(1){
		if(shared->STOP) break;
		if(run == 0) break;


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
		int size = snprintf(NULL,0,"[%s]\tdata=%d\n",time_buffer,shared->data);
		char * buffer = (char * ) malloc( (size+1) * sizeof(char));
		int check = sprintf(buffer,"[%s]\tdata=%d\n",time_buffer,shared->data);

		if( check != size ){
			printf("Couldn't write properly\n");
		}

		// "Ecriture" du message
		Message to_send;
		to_send.ptr = buffer;
		to_send.length = size;

		// Ecriture dans le pipe
		ipc_status_t status = write_msg( shared->pipe[1], &to_send);

		// Increment NB_MESSAGE_ENV
		shared->NB_MESSAGE_ENV++;

		switch (status)
		{
			case PIPE_OK :
				run = 1;
				pthread_cond_signal(&shared->data_ready);
				break;
			case PIPE_CLOSED :
				run = 0;
				break;
			case PIPE_ERROR :
				printf("\nPIPE ERROR\n");
				run = 0;
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

	printf("Closing add  TRHEAD\n");
	pthread_exit(NULL);
}

void * worker_log(void * arg){
	Configuration * shared = (Configuration * ) arg;

	int run = 1;
	while( run == 1){

		Message received;
		// Lecture du pipe
		ipc_status_t status = read_msg( shared->pipe[0], &received);
		char * buffer = (char *) received.ptr;
		char to_log[received.length];
		memset(to_log,0,sizeof(to_log));
		switch (status)
		{
			case PIPE_OK:
				run = 1;
				pthread_mutex_lock( &(shared->MUTEX) );
				pthread_cond_wait( &(shared->data_ready),&(shared->MUTEX) );
				if(shared->STOP == 1){
					run = 0;
					break;
				}

				// Increment NB_MESSAGE_REC
				shared->NB_MESSAGE_REC++;
		
				for(int i = 0 ; i < received.length ; i++){
					to_log[i] = buffer[i];
				}

				// Ecriture dans le log
				fputs(to_log,shared->file);
				if(shared->flush_log == 1)
					fflush(shared->file);

				pthread_mutex_unlock( &(shared->MUTEX) );
				// Free pour éviter les leaks
				free(received.ptr);

				break;
			case PIPE_EOF:
				run = 0;
				//printf("Received STOP == 1. Closing Pipe.\n");
				break;
			case PIPE_ERROR:
				// log erreur dans le futur
				run = 0;
				break;
			default:
				printf("OUT OF RANGE\n");
				run = 0;

		}

	}

	printf("Closing log THREAD\n");
	pthread_exit(NULL);
}

void * worker_heartbeat(void * arg){
	Configuration * shared = (Configuration * ) arg;
	struct timespec rec_heartbeat = {(shared->freq_heartbeat/1000) , (shared->freq_heartbeat%1000) * pow(10,6)};

	float t_since_start = 0;
	while( shared->STOP != 1){
		t_since_start += (rec_heartbeat.tv_sec + rec_heartbeat.tv_nsec/pow(10,9));

		if(shared->enable_show == 1)
		{
		printf("Time since start = %f.\n",t_since_start);
		pthread_mutex_lock( &(shared->MUTEX) );

		printf("%d messages sent.\n",shared->NB_MESSAGE_ENV);
		printf("%d messages received\n",shared->NB_MESSAGE_REC);
		
		(shared->NB_MESSAGE_ENV != shared->NB_MESSAGE_REC) ? printf("nb msg sent != nb msg received\n") :printf("Link OK\n\n") ;

		pthread_mutex_unlock( &(shared->MUTEX) );
		}
		nanosleep(&rec_heartbeat,NULL);

	}
	printf("Closing heart TRHEAD\n");
	pthread_exit(NULL);
}

void * worker_fifo(void * arg){
	Configuration * shared = (Configuration * ) arg;

	// Open FIFO
	shared->fifo = open(shared->fifo_path, O_RDONLY | O_NONBLOCK);

	char * line;

	while( shared->STOP != 1 ){
		struct pollfd pfd = {shared->fifo, POLLIN, 0};
		int res = poll(&pfd,1,500); // entree : struct pollfd, nb de descripeturs dans la struct, temps d'attente
		if( res > 0 ){
			//FILE * fifo_fd = fdopen( shared->fifo, "r");
			
			char character;
			int len;
			int index = 0;
			line = (char *) malloc( sizeof(char) * 20 );	
			if( line == NULL )
				exit(EXIT_FAILURE);

			while( len = read(shared->fifo,&character,1) > 0 ){
				line[index++] = character;
			}

			line = (char *) realloc(line,sizeof(char)*index);

			if( len == 0 ){
				close(shared->fifo);
				shared->fifo = open(shared->fifo_path, O_RDONLY | O_NONBLOCK);
			}
				

			if( len < 0 )
				exit(EXIT_FAILURE);

			//printf("%s",line);

			pthread_mutex_lock( &(shared->MUTEX) );
			if( strcmp(line,"enable_show\n") == 0){
				shared->enable_show = 1;
			}else if( strcmp(line,"disable_show\n") == 0){
				shared->enable_show = 0;
			}else if( strcmp(line,"stop\n") == 0){
				shared->STOP = 1;
				if( pthread_kill(signal_handler_tid,SIGTERM) ){
					printf("Couldn't kill signal handler\n");
					exit(EXIT_FAILURE);
				}
			}else if (strcmp(line,"flush_on\n") == 0){
				shared->flush_log = 1;
			}else if (strcmp(line,"flush_off\n") == 0){
				shared->flush_log = 0;
			}else{
				printf("Command not recognized.\n");
				sleep(3);
			}
			pthread_mutex_unlock( &(shared->MUTEX) );
			free(line);
		}else if( res == 0 ){
		}else{
			break;
		}	

	}

	//Free and Close FIFO
	free(line);
	close(shared->fifo);
	printf("Closing Fifo TRHEAD\n");
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
