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

typedef enum{
	PIPE_OK,
	PIPE_EOF,
	PIPE_CLOSED,
	PIPE_ERROR,
	UNKNOWN
}ipc_status_t;

ipc_status_t write_msg(int pipe_write_id, Message * to_send);
ipc_status_t read_msg(int pipe_read_id, Message * received);


#endif
