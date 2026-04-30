#include "misc.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

ipc_status_t write_msg(int pipe_write_id, Message * to_send ){
	int res = write( pipe_write_id, to_send, sizeof(Message) );
	if(res > 0){
		return PIPE_OK;
	}else if(res + errno == EPIPE){
		return PIPE_CLOSED;
	}else if(res < 0){
		return PIPE_ERROR;
	};
	return UNKNOWN;
}

ipc_status_t read_msg(int pipe_read_id, Message * received){
	int res = read( pipe_read_id, received, sizeof(Message) );
	if(res > 0){
		return PIPE_OK;
	}else if(res == 0){
		return PIPE_EOF;
	}else if(res < 0){
		return PIPE_ERROR;
	}
	return UNKNOWN;
};

