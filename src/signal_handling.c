#define _POSIX_C_SOURCE 200809L

#include "signal_handling.h" // Entre guillemets car recherche dans le repertoire le plus proche qui s'appelle include
#include "misc.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

volatile sig_atomic_t stop;
pthread_t signal_handler_tid;

void init_signal(void){
	stop = 0;

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set,SIGINT);
	sigaddset(&set,SIGTERM);

	pthread_sigmask(SIG_BLOCK,&set,NULL);
	
	if(pthread_create(&signal_handler_tid,NULL,*signal_handler,NULL) != 0){
		printf("Couldn't create signal pthread.\n");
		exit(EXIT_FAILURE);
	}

	printf("Signals were correctly handled.\n");
}

void * signal_handler(void * ){
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set,SIGINT);
	sigaddset(&set,SIGTERM);

	int sig;
	sigwait(&set,&sig);

	if( sig == SIGINT || sig == SIGTERM )
		stop = 1;

	pthread_exit(NULL);
};

int check_stop_requested(void){
	return stop;
}

void wait_for_ending(void){
	if(pthread_join(signal_handler_tid,NULL) != 0){
		printf("Couldn't join signal pthread.\n");
		exit(EXIT_FAILURE);
	}
		
	printf("\nTermination performed successfully.\n");
}
