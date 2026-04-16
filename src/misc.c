#include "misc.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void write_str(int pipe_id, char * str ){
	if( write(pipe_id, str, strlen(str) ) == -1 ){
		printf("Couldn't write\n");
		exit(EXIT_FAILURE);
	}
}

void read_str(int pipe_id[2], char * buffer){
	close(pipe_id[1]);
	while( read(pipe_id[0],buffer,1) > 0 ){

	}
	close(pipe_id[0]);
}
