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

	ArgThread * Entree = (ArgThread * ) arg;

	static char time_buffer[TIME_LENGTH];
	time_t rawtime;

	int run = 1;
	while(1){
		// Race Protection
		pthread_mutex_lock( &(Entree->Etat->MUTEX) );

		if(Entree->Etat->StopFlag){
			pthread_mutex_unlock( &(Entree->Etat->MUTEX) );
			break;
		}

		if(run == 0){
			pthread_mutex_unlock( &(Entree->Etat->MUTEX) );
			break;
		}


	       	// Attribue le temps depuis le 1er Janvier 1970 dans  la variable
		time( &rawtime );
	       	// Transforme la durée en temps normal à notre période
		struct tm * timeinfo = localtime( &rawtime );
		// Formatage
		strftime(time_buffer,TIME_LENGTH,"%d/%m/%Y %H:%M:%S",timeinfo);

		// Increment
		Entree->Etat->Data++;

		// Generation du texte
		int size = snprintf(NULL,0,"[%s]\tData=%d\n",time_buffer,Entree->Etat->Data);
		char * buffer = (char * ) malloc( (size+1) * sizeof(char));
		int check = sprintf(buffer,"[%s]\tData=%d\n",time_buffer,Entree->Etat->Data);

		if( check != size ){
			printf("Couldn't write properly\n");
		}

		// "Ecriture" du message
		Message to_send;
		to_send.ptr = buffer;
		to_send.length = size;

		// Ecriture dans le pipe
		ipc_status_t status = write_msg( Entree->IPC->Pipe_fd[1], &to_send);

		// Increment NombreMessageEnvoye
		Entree->Etat->NombreMessageEnvoye++;

		switch (status)
		{
			case PIPE_OK :
				run = 1;
				pthread_cond_signal(&Entree->Etat->DataReady);
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
		pthread_mutex_unlock( &(Entree->Etat->MUTEX) );

		// Périodicicté
		nanosleep(&rec_add,NULL);
	}

	printf("Closing add  TRHEAD\n");
	pthread_exit(NULL);
}

void * worker_log(void * arg){
	ArgThread * Entree = (ArgThread * ) arg;

	int run = 1;
	while( run == 1){

		Message received;
		// Lecture du pipe
		ipc_status_t status = read_msg( Entree->IPC->Pipe_fd[0], &received);
		char * buffer = (char *) received.ptr;
		char * to_log = (char *) malloc( sizeof(char) * (received.length + 1) );
	//	memset(to_log,0,sizeof(to_log));
		switch (status)
		{
			case PIPE_OK:
				run = 1;
				pthread_mutex_lock( &(Entree->Etat->MUTEX) );
				pthread_cond_wait( &(Entree->Etat->DataReady),&(Entree->Etat->MUTEX) );
				if(Entree->Etat->StopFlag == 1){
					run = 0;
					free(to_log);
					break;
				}

				// Increment NombreMessageRecu
				Entree->Etat->NombreMessageRecu++;
		
				for(int i = 0 ; i < received.length ; i++){
					to_log[i] = buffer[i];
				}
				to_log[received.length] = '\0';

				// Ecriture dans le log
				fputs(to_log,Entree->IPC->LogFile);
				if(Entree->Etat->FlushLog == 1)
					fflush(Entree->IPC->LogFile);

				pthread_mutex_unlock( &(Entree->Etat->MUTEX) );
				// Free pour éviter les leaks
				free(to_log);

				break;
			case PIPE_EOF:
				run = 0;
				free(to_log);
				//printf("Received StopFlag == 1. Closing Pipe.\n");
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
	ArgThread * Entree = (ArgThread * ) arg;
	struct timespec rec_heartbeat = {(Entree->Configuration->FreqHeartbeat/1000) , (Entree->Configuration->FreqHeartbeat%1000) * pow(10,6)};

	float t_since_start = 0;
	while(1){
		pthread_mutex_lock( &(Entree->Etat->MUTEX) );
		if(Entree->Etat->StopFlag){
			pthread_mutex_unlock( &(Entree->Etat->MUTEX) );
		       	break;
		}

		t_since_start += (rec_heartbeat.tv_sec + rec_heartbeat.tv_nsec/pow(10,9));

		if(Entree->Etat->EnableShow == 1)
		{
		printf("Time since start = %f.\n",t_since_start);

		printf("%d messages sent.\n",Entree->Etat->NombreMessageEnvoye);
		printf("%d messages received\n",Entree->Etat->NombreMessageRecu);
		
		(Entree->Etat->NombreMessageEnvoye != Entree->Etat->NombreMessageRecu) ? printf("nb msg sent != nb msg received\n") :printf("Link OK\n\n") ;

		}
		pthread_mutex_unlock( &(Entree->Etat->MUTEX) );

		nanosleep(&rec_heartbeat,NULL);

	}
	printf("Closing heart TRHEAD\n");
	pthread_exit(NULL);
}

void * worker_fifo(void * arg){
	ArgThread * Entree = (ArgThread * ) arg;

	// Open FIFO
	Entree->IPC->Fifo_fd = open(Entree->IPC->FifoPath, O_RDONLY | O_NONBLOCK);

	char * line;

	while( 1 ){
		pthread_mutex_lock( &(Entree->Etat->MUTEX) );
		if(Entree->Etat->StopFlag){
			pthread_mutex_unlock( &(Entree->Etat->MUTEX) );
		       	break;
		}
		pthread_mutex_unlock( &(Entree->Etat->MUTEX) );

		struct pollfd pfd = {Entree->IPC->Fifo_fd, POLLIN, 0};
		int res = poll(&pfd,1,500); // entree : struct pollfd, nb de descripeturs dans la struct, temps d'attente
		if( res > 0 ){
			//FILE * IPC->Fifo_fd_fd = fdopen( Entree->IPC->Fifo_fd, "r");
			
			char character;
			int len = 1;
			int index = 0;
			line = (char *) malloc( sizeof(char) * 20 );	
			if( line == NULL )
				exit(EXIT_FAILURE);

			while( len > 0 ){
				len = read(Entree->IPC->Fifo_fd,&character,1);
				line[index++] = character;
			}

			line = (char *) realloc(line,sizeof(char)*index);
			line[strcspn(line,"\r\n")] = '\0';

			if( len == 0 ){
				close(Entree->IPC->Fifo_fd);
				Entree->IPC->Fifo_fd = open(Entree->IPC->FifoPath, O_RDONLY | O_NONBLOCK);
			}
				

			if( len < 0 )
				exit(EXIT_FAILURE);

			//printf("%s",line);

			pthread_mutex_lock( &(Entree->Etat->MUTEX) );
			if( strcmp(line,"EnableShow") == 0){
				Entree->Etat->EnableShow = 1;
			}else if( strcmp(line,"DisableShow") == 0){
				Entree->Etat->EnableShow = 0;
			}else if( strcmp(line,"Stop") == 0){
				Entree->Etat->StopFlag = 1;
				if( pthread_kill(signal_handler_tid,SIGTERM) ){
					printf("Couldn't kill signal handler\n");
					exit(EXIT_FAILURE);
				}
			}else if (strcmp(line,"flush_on") == 0){
				Entree->Etat->FlushLog = 1;
			}else if (strcmp(line,"flush_off") == 0){
				Entree->Etat->FlushLog = 0;
			}else{
				printf("Command not recognized.\n");
			}
			pthread_mutex_unlock( &(Entree->Etat->MUTEX) );
			free(line);
		}else if( res == 0 ){
		}else{
			free(line);
			break;
		}	

	}

	//Free and Close FIFO
	close(Entree->IPC->Fifo_fd);
	printf("Closing Fifo TRHEAD\n");
	pthread_exit(NULL);
}

/* Was Only Here During Developping Phase
void * worker_show(void * arg){
	struct timespec rec_show = {1,0};
	ArgThread * Entree = (ArgThread * ) arg;

	while( Entree->Etat->StopFlag != 1 ){
		pthread_mutex_lock( &(Entree->Etat->MUTEX) );

		//printf("%d in shared increment\n",Entree->Etat->Data);
		
		pthread_mutex_unlock( &(Entree->Etat->MUTEX) );
		nanosleep(&rec_show,NULL);
	}

	pthread_exit(NULL);
}
*/

