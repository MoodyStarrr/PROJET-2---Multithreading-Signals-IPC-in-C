#ifndef MISC_H
#define MISC_H
#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct{
	char * ConfigFilePath;
	char * ConfigFormat;
	int NombreWorkerAdd;
	int FreqHeartbeat;
}Config;

typedef struct{
	int Data;
	int StopFlag;
	int EnableShow;
	int FlushLog;
	int NombreMessageEnvoye;
	int NombreMessageRecu;
	pthread_mutex_t MUTEX;
	pthread_cond_t DataReady;
}RuntimeState;

typedef struct {
	FILE * LogFile;
	char * FifoPath;
	int Fifo_fd;
	int Pipe_fd[2];
}IpcHandles;

typedef struct {
	Config * Configuration;
	RuntimeState * Etat;
	IpcHandles * IPC;
}ArgThread;

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
int parse_conf(Config * Configuration, RuntimeState * Etat, IpcHandles * IPC);


#endif
