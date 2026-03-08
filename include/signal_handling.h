#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

void init_signal(void);

void * signal_handler(void * );

int check_stop_requested(void);

void wait_for_ending(void);

#endif

