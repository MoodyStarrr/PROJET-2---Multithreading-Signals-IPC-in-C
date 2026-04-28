#include "misc.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void write_msg(int pipe_write_id, Message * to_send ){
	if( write(pipe_write_id, to_send, sizeof(Message) ) == -1 ){
		printf("\nCouldn't write\n");
		exit(EXIT_FAILURE);
	}
}
