#include "misc.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

ipc_status_t write_msg(int pipe_write_id, Message * to_send ){
	int res = write( pipe_write_id, to_send, sizeof(Message) );
	if(res > 0){
		return PIPE_OK;
	}else if(res < 0 && errno == EPIPE){
		return PIPE_CLOSED;
	}else if(res < 0){
		return PIPE_ERROR;
	};
	return UNKNOWN;
}

ipc_status_t read_msg(int pipe_read_id, Message * received){
	int res = read( pipe_read_id, received, sizeof(* received) );
	if(res > 0){
		return PIPE_OK;
	}else if(res == 0){
		return PIPE_EOF;
	}else if(res < 0){
		return PIPE_ERROR;
	}
	return UNKNOWN;
}

int parse_conf(Configuration * shared){
	shared->data = 0;
	shared->STOP = 0;
	shared->NB_MESSAGE_REC = 0;
	shared->NB_MESSAGE_ENV = 0;

	FILE * conf_file = fopen("systemd/projet_2_app.conf","r+");
	char * line = NULL;
	size_t len;

	while(getline(&line,&len,conf_file) != -1){
		char * token = strtok(line,"=");

		if( strcmp(token,"log_path") == 0 ){
			token = strtok(NULL,"\n");
			int size_path_log = snprintf(NULL,0,token);
			shared->file_path = (char *) malloc(sizeof(char) * size_path_log);
			memcpy(shared->file_path,token, sizeof(char) * size_path_log);
		}else if( strcmp(token,"log_format") == 0 ){
			token = strtok(NULL,"\n");
			int size_log_format = snprintf(NULL,0,token);
			shared->format = (char *) malloc(sizeof(char) * size_log_format);
			memcpy(shared->format,token,sizeof(char) * size_log_format);
		}else if( strcmp(token,"period_ms") == 0 ){
			token = strtok(NULL,"\n");
			int value = (int) strtol(token,(char **) NULL,10);
			shared->freq_heartbeat = value;
		}else if( strcmp(token,"flush_log") == 0 ){
			token = strtok(NULL,"\n");
			int value = (int) strtol(token,(char **) NULL,10);
			shared->flush_log = value;
		}else if( strcmp(token,"NB_WORKER_ADD") == 0 ){
			token = strtok(NULL,"\n");
			int value = (int) strtol(token,(char **) NULL,10);
			shared->NB_WORKER_ADD = value;
		}else{
			//printf("%s is not part of configuration.\n",token);
		}		
	}

	shared->file = fopen(shared->file_path,"a+");

	fclose(conf_file);
	free(line);
	return EXIT_SUCCESS;
}

