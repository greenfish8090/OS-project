#ifndef C_H
#define C_H

#include <pthread.h>

#define WRITE_END 1
#define READ_END 0

typedef struct sh_seg {
	pthread_condattr_t instr_cond_attr;
	pthread_mutexattr_t instr_mutex_attr;
	pthread_cond_t instr_ready_cond;
	pthread_mutex_t instr_ready_mutex;
	char executing;
	char waiting;
	char finished;
	char sched_instr[10];
} sh_seg_t;

void monitor(int c);

#endif
