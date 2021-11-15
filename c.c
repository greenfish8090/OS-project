#include "c.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <pthread.h>
#include <time.h>

/// mutex initalized to the default value
pthread_mutex_t sched_mutex = PTHREAD_MUTEX_INITIALIZER;
/// condition value initalized to the default value
pthread_cond_t sched_cond = PTHREAD_COND_INITIALIZER;

/// variable defining the sleep state requirement for the process
char should_sleep = 1;
/// variable defining the current sleep state for the process
char sleeping = 0;
/// type of process being done
char proc_type;
/// write pipe descriptor
int write_pipe;
/// integer input for the process 'i'
int ni;

unsigned long long sum = 0; // This is only used for C1 and C3

int num; // The current number to be added to sum. For C1 this is just i in the loop. For C2 and C3 this is read from file.

// These variables are only used for C2 and C3
FILE* file; 
char file_path[10];

float cpu_time = 0;
clock_t start;
clock_t end;
int started = 0;

/// Function that executes the actual job to be done by the process
/** This function is run by pthread_create to execute whatever instruction a process is supposed to do.
 * Using a switch case to check which process is running, it then performs the instructions and stops.
 * the main function montiors the process using shared memory.
 * 
 * @returns NULL pointer for use in pthread_create()
 * @param ptr the argument provided by pthread_create to the function: in this case, whether the process has finished execution or not
 */
void* job(void *ptr) {
	char *finished = (char*) ptr;

	//We are detaching the thread here to make the thread independent of any join() functions
	pthread_detach(pthread_self());
	
	//Critical section: all execution processes here
	pthread_mutex_lock(&sched_mutex);

	if (proc_type != '0') //Only open file if process is C2 or C3
		file = fopen(file_path, "r");

	for(int i = 0; i <= ni; i++) {

		//If the process is designated to be sleeping
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

		//Actual execution of each process
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

	// Time calculations
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

/// The main function performing the given task based on the process type
/** assigns pipes and initializes shared memory for the given process, then creates
 * a thread to perform the task at hand. It also monitors the thread during runtime.
 */
int main(int argc, char** args) {

	//Assign all argument variables to the appropriate variables
	write_pipe = atoi(args[1]);
	proc_type = args[3][0];
	ni = atoi(args[4]);
	if (proc_type != '0') // If this is C2 or C3 we need the file path
		strcpy(file_path, args[5]);
	
	//Start shared memory initialization
	key_t shmkey = atoi(args[2]);
	int shmid = shmget(shmkey, 1024, 0666 | IPC_CREAT);
	sh_seg_t *shseg = (sh_seg_t*) shmat(shmid, (void*)0, 0);
	
	//Initialize new thread beginning the actual job; the shseg-> finished variable is affected by job()
	pthread_t tid;
	pthread_create(&tid, NULL, &job, &shseg->finished);
	while(!sleeping);
	
	char breakflag = 1;
	
	while(breakflag) {		

		//Critical section: monitoring execution on all instructions using locks		
		
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
	
	//end shared memory segment
	shmdt(shseg);
	
	return 0;
}
