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
