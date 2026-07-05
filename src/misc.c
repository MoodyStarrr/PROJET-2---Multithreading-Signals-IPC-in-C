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

int parse_conf(Config * Configuration, RuntimeState * Etat)
	Etat->Data = 0;
	Etat->EnableShow = 0;
	Etat->StopFlag = 0;
	Etat->NombreMessageRecu = 0;
	Etat->NombreMessageEnvoye = 0;

	FILE * ConfigFile = fopen("systemd/projet_2_app.conf","r+");
	if (ConfigFile == NULL){
		printf("Couldn't open configuration file");
		exit(EXIT_FAILURE);
	}
	char * line = NULL;
	size_t len;

	while(getline(&line,&len,ConfigFile) != -1){
		char * token = strtok(line,"=");

		if( strcmp(token,"log_path") == 0 ){
			token = strtok(NULL,"\n");
			int size_path_log = strlen(token);
			Etat->file_path = (char *) malloc(sizeof(char) * size_path_log);
			memcpy(Etat->file_path,token, sizeof(char) * size_path_log);
		}else if( strcmp(token,"log_format") == 0 ){
			token = strtok(NULL,"\n");
			int size_log_format = strlen(token);
			Etat->format = (char *) malloc(sizeof(char) * size_log_format);
			memcpy(Etat->format,token,sizeof(char) * size_log_format);
		}else if( strcmp(token,"period_ms") == 0 ){
			token = strtok(NULL,"\n");
			int value = (int) strtol(token,(char **) NULL,10);
			Etat->freq_heartbeat = value;
		}else if( strcmp(token,"flush_log") == 0 ){
			token = strtok(NULL,"\n");
			int value = (int) strtol(token,(char **) NULL,10);
			Etat->flush_log = value;
		}else if( strcmp(token,"NB_WORKER_ADD") == 0 ){
			token = strtok(NULL,"\n");
			int value = (int) strtol(token,(char **) NULL,10);
			Etat->NB_WORKER_ADD = value;
		}else{
			//printf("%s is not part of configuration.\n",token);
		}		
	}

	Etat->file = fopen(Etat->file_path,"a+");

	fclose(ConfigFile);
	free(line);
	return EXIT_SUCCESS;
}

