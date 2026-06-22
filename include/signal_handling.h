#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#define _POSIX_C_SOURCE 200809L
#include <pthread.h>

extern pthread_t signal_handler_tid;

void init_signal(void);

void * signal_handler(void * );

int check_stop_requested(void);

void wait_for_ending(void);

#endif

