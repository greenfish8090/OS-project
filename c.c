#include "c.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <pthread.h>
#include <time.h>

pthread_mutex_t sched_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sched_cond = PTHREAD_COND_INITIALIZER;

char should_sleep = 1;
char sleeping = 0;
char proc_type;
int write_pipe;
int ni;
unsigned long long sum = 0; // This is only used for C1 and C3
int num; // The current number to be added to sum. For C1 this is just i in the loop. For C2 and C3 this is read from file.
FILE* file; // This is only used for C2 and C3
char file_path[10]; // ^^
float cpu_time = 0;
clock_t start;
clock_t end;
int started = 0;

void* job(void *ptr) {
	char *finished = (char*) ptr;
	pthread_detach(pthread_self());
	
	pthread_mutex_lock(&sched_mutex);
	if (proc_type != '0') //Only open file if process is C2 or C3
		file = fopen(file_path, "r");
	for(int i = 0; i <= ni; i++) {
		if(should_sleep) {
			should_sleep = 0;
			sleeping = 1;

			if(started) {
				end = clock();
				cpu_time += (float)(end - start) / (CLOCKS_PER_SEC / 1000.0f);
			}

			pthread_cond_wait(&sched_cond, &sched_mutex);
			started = 1;
			start = clock();
			sleeping = 0;
		}

		switch (proc_type)
		{
			case '0':
			// This is C1. Add numbers 1 to ni and store in sum
				sum += (unsigned long long)i;
				break;
			
			case '1':
			// This is C2. Read ni numbers from file and print to console
				fscanf(file, "%d", &num);
        		printf("  In C2: %d\n", num);
				break;

			case '2':
			// This is C3. Read ni numbers from file and add them up
				fscanf(file, "%d", &num);
        		sum += (unsigned long long)num;
				break;
		}

	}
	end = clock();
	cpu_time += (float)(end - start) / (CLOCKS_PER_SEC / 1000.0f);
	printf("Total cpu time for process C%c: %.3f\n", proc_type - '0' + '1', cpu_time);
	// Process writes sum to pipe. Note that any process writes this sum only when it's done with its task, so for C2 this
	// can be interpreted as "Done printing".
	if (proc_type == '1') {
		write(write_pipe, "Done printing", 14);
	}
	else
		write(write_pipe, &sum, sizeof(sum));
	close(write_pipe);
	*finished = 1;
	pthread_mutex_unlock(&sched_mutex);
	
	return NULL;
}

int main(int argc, char** args) {
	write_pipe = atoi(args[1]);
	proc_type = args[3][0];
	ni = atoi(args[4]);
	if (proc_type != '0') // If this is C2 or C3 we need the file path
		strcpy(file_path, args[5]);
	
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
