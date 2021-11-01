#include "c.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <pthread.h>

pthread_mutex_t sched_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sched_cond = PTHREAD_COND_INITIALIZER;

char should_sleep = 1;
char sleeping = 0;
char proc_type;

void* job(void *ptr) {
	char *finished = (char*) ptr;
	pthread_detach(pthread_self());
	
	pthread_mutex_lock(&sched_mutex);
	for(int i = 0; i < 100000000; i++) {
		if(should_sleep) {
			should_sleep = 0;
			sleeping = 1;
			pthread_cond_wait(&sched_cond, &sched_mutex);
			sleeping = 0;
		}
		
		//printf("%d %c\n", i, proc_type);
		//fflush(stdout);
	}
	
	*finished = 1;
	pthread_mutex_unlock(&sched_mutex);
	
	return NULL;
}

int main(int argc, char** args) {
	int write_pipe = atoi(args[1]);
	proc_type = args[3][0];
	
	char* msg = (char*) malloc(sizeof(char)*50);
	sprintf(msg, "H");
	write(write_pipe, msg, strlen(msg));
	close(write_pipe);
	
	key_t shmkey = atoi(args[2]);
	int shmid = shmget(shmkey, 1024, 0666 | IPC_CREAT);
	sh_seg_t *shseg = (sh_seg_t*) shmat(shmid, (void*)0, 0);
	
	pthread_t tid;
	pthread_create(&tid, NULL, &job, &shseg->finished);
	while(!sleeping);
	
	char breakflag = 1;
	
	while(breakflag) {				
		pthread_mutex_lock(&shseg->instr_ready_mutex);
		
		shseg->waiting = 1;
		pthread_cond_wait(&shseg->instr_ready_cond, &shseg->instr_ready_mutex);
		shseg->waiting = 0;
		shseg->executing = 1;
		
		if(strcmp(shseg->sched_instr, "sleep") == 0 && !sleeping) {
			should_sleep = 1;
			pthread_mutex_lock(&sched_mutex);
			pthread_mutex_unlock(&sched_mutex);
		}
		else if(strcmp(shseg->sched_instr, "start") == 0 && sleeping) {
			pthread_mutex_lock(&sched_mutex);
			pthread_cond_signal(&sched_cond);
			pthread_mutex_unlock(&sched_mutex);
		}
		else if(strcmp(shseg->sched_instr, "finish") == 0) breakflag = 0;
		
		shseg->executing = 0;
		pthread_mutex_unlock(&shseg->instr_ready_mutex);
	}
	
	shseg->waiting = 1;
	
	shmdt(shseg);
	
	return 0;
}
