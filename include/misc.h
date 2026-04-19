#ifndef MISC_H
#define MISC_H
#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
	FILE * file;
	char * file_path;
	char * format;
	int STOP;
	int data;
	int NB_MESSAGE;
	int freq_heartbeat;
	int pipe[2];
	pthread_mutex_t MUTEX;
}Configuration;

typedef struct {
	void * ptr;
	int length;
}Message;

void write_msg(int pipe_id, Message * to_send);


#endif
