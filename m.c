/*
Members:                       IDs:

Yash Shah                      2019A7PS0102H
Arjav Garg                     2019A7PS0068H
Subienay Ganesh                2019A7PS0096H
Nayan Sharma                   2019A7PS0114H
Shah Aayush Keval              2019A7PS0137H
Chaudhari Nisarg Sanjaykumar   2019A7PS0176H
Pranav Balaji                  2019A7PS0040H
Ayush Pote                     2019A7PS0112H

*/
#include "c.h"
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#define BILLION  1000000000L

/// Returns the cuurent time in nanoseconds
/** This function uses clock_gettime to fetch the current time from the beginning of the epoch to calculate 
 * the current time in nanoseconds, using the timespec struct to convert the data into a uint64_t value.
 * @returns a uint64_t value representing the current time in nanoseconds
 */
uint64_t nanotime(void)
{
	long int ns;
	uint64_t all;
	time_t sec;
	struct timespec spec;

	clock_gettime(CLOCK_REALTIME, &spec);
	sec = spec.tv_sec;
	ns = spec.tv_nsec;

	all = (uint64_t) sec * BILLION + (uint64_t) ns;

	return all;
}

/// Struct defining process variables
/** Linked-list type struct defining all process relevant variables such as pid,
 * pipes and shared memory info with sh_seg_t.
 */
typedef struct proc_list {
	pid_t pid;
	int proc_type;
	int read_pipe;
	sh_seg_t *shseg;
	int shmid;
	struct proc_list* next;
	
} plist_t;

/// 
plist_t *p_list, *tail;

/// Function converting integers to string form
/** This function converts integer values to a string format using snprintf
 * 
 * @returns a char* pointer pointing to the string
 * @param x the input integer to convert
 */
char* itos(int x) {
	int length = snprintf(NULL, 0, "%d", x);
	char* str = malloc(sizeof(int) + 1);
	snprintf(str, length + 1, "%d", x );
	return str;
}

/// Enum containing the types of scheduling that can be used
typedef enum sched {
	RR, FCFS
} sch_alg;

/// Arrival Times for each process
long int arrival_times[3];
/// Time at which each process last slept
long int last_sleep_times[3];
/// Waiting Times for each process
long int waiting_times[3];
/// Turnaround Times for each process
long int turn_around_times[3];

/// Function to send instructions to a process 
/** This function sends the given instruction to the given process and checks if
 * the process has completed its job. If it has, it returns a signal stating the same,
 * or alternatively continues its job.
 * 
 * @returns 1 if the process given is finished, or 0 otherwise
 * @param pp_list a double pointer pointing to the process to which the instruction is to be sent
 * @param str the instruction to be sent to pp_list
 */
int send_instruction(plist_t **pp_list, char *str) {
	plist_t *p_list = *pp_list;

	//Wait for process to stop waiting
	while(!p_list->shseg->waiting);
	
	//Check for finished state
	char finished = 0;
	if(p_list->shseg->finished) {
		strcpy(str, "finish");
		finished = 1;
	}
	
	//Critical section: performing instruction
	pthread_mutex_lock(&p_list->shseg->instr_ready_mutex);
	p_list->shseg->executing = 1;
	
	strcpy(p_list->shseg->sched_instr, str);
	
	pthread_cond_signal(&p_list->shseg->instr_ready_cond);
	pthread_mutex_unlock(&p_list->shseg->instr_ready_mutex);

	//wait for process to be done with execution
	while(p_list->shseg->executing);
	
	// Has the process responded with a finished flag?
	if(finished) {
		//printf("finished\n");
		shmdt(p_list->shseg);
		shmctl(p_list->shmid, IPC_RMID, NULL);

		if(p_list->proc_type != 1) {
			unsigned long long sum;
			read(p_list->read_pipe, &sum, sizeof(sum));
			printf("C%c ended. Sum is %lld\n", p_list->proc_type+'1', sum);
		}
		else {
			char buffer[15];
			read(p_list->read_pipe, buffer, 14);
			printf("C2 ended. %s\n", buffer);
		}

		plist_t *prev = p_list;
		while(prev->next != p_list) prev = prev->next;
		
		prev->next = p_list->next;
		free(p_list);
		
		*pp_list = prev->next;
		return 1;
	}
	
	//Process not finished yet
	return 0;
}

/// The main function where the arguments are processed and the process and scheduling is initialized
/** @param argc Number of arguments
 * @param argv String array containing the arguments
 */
int main(int argc, char* argv[]) {

	long quantum = 0;
	char* ptr_temp;

	// Wrong argument format
	if (argc!=7 && argc!=8) {
		fprintf(stderr, "Usage: ./m.out n1 n2 n3 alg f2 f3 time_quantum_ns\n");
		return -1;
	}
	sch_alg sh;
	if (argv[4][0] == 'R') {
		sh = RR;
		quantum = strtol(argv[7], &ptr_temp, 10);
	}
	else
		sh = FCFS;

	p_list = NULL;
	pid_t last_created = getpid();
	int is_child = 0;

	//Create all processes and associated threads
	for(int i = 0; i < 3; i++) {
		if(last_created) {
			plist_t* new_proc;
			if(!p_list) {
				new_proc = p_list = tail = (plist_t*) malloc(sizeof(plist_t));
				p_list->next = tail;
			}
			else {
				new_proc = (plist_t*) malloc(sizeof(plist_t));
				new_proc->next = p_list;
				tail->next = new_proc;
				tail = tail->next;
			}
			
			//Assign all variables in plist_t and sh_seg_t
			
			new_proc->pid = last_created;
			
			int p[2];
			pipe(p);
			new_proc->read_pipe = p[READ_END];
			
			key_t shmkey = ftok("m.c", i);
			int shmid = shmget(shmkey, 1024, 0666 | IPC_CREAT);
			sh_seg_t *shseg = shmat(shmid, (void*)0, 0);
			new_proc->shseg = shseg;
			new_proc->shmid = shmid;
			new_proc->proc_type = i;

			pthread_mutexattr_init(&shseg->instr_mutex_attr);
			pthread_mutexattr_setpshared(&shseg->instr_mutex_attr, PTHREAD_PROCESS_SHARED);
			pthread_mutex_init(&shseg->instr_ready_mutex, &shseg->instr_mutex_attr);
			pthread_mutexattr_destroy(&shseg->instr_mutex_attr);
			
			pthread_condattr_init(&shseg->instr_cond_attr);
			pthread_condattr_setpshared(&shseg->instr_cond_attr, PTHREAD_PROCESS_SHARED);
			pthread_cond_init(&shseg->instr_ready_cond, &shseg->instr_cond_attr);
			pthread_condattr_destroy(&shseg->instr_cond_attr);

			shseg->executing = 1;
			shseg->waiting = 0;
			
			//We use execvp to execute c.c for each process.
			//execvp() takes the filename to execute and an argument string array as input to execute the file with the given arguments.

			last_created = fork();
			if(!last_created) {
				close(p[READ_END]);
				// Command: ./c <fd_write> <shmkey> <proc> <nx> for C1
				// Command: ./c <fd_write> <shmkey> <proc> <nx> <file_path> for C2 and C3
				char* args1[] = {"./c", itos(p[WRITE_END]), itos(shmkey), itos(i), argv[i+1], NULL};
				char* args23[] = {"./c", itos(p[WRITE_END]), itos(shmkey), itos(i), argv[i+1], argv[i+4], NULL};
				if(i) // If C2 or C3
					execvp(args23[0], args23);
				execvp(args1[0], args1);
			}
			else {
				close(p[WRITE_END]);
			}
		}
	}
	
	//time quantum of an Round Robin algorithm (if used)
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = quantum;
	
	int total = 3;
	int first = 1;
	unsigned long first_time;
	int ptype_temp;
	
	//Start with program execution, process by process based on the given scheduling algorithm
	while(total) {
		char str[10];

		if(first) {
			first_time = nanotime();
			for(int i=0; i<3; ++i) {
				arrival_times[i] = first_time;
				last_sleep_times[i] = first_time;
			}
			first = 0;
		}

		printf("C%c starts at %.3f\n", p_list->proc_type+'1', (float)(nanotime() - first_time)/1e6);

		strcpy(str, "start");
		waiting_times[p_list->proc_type] += nanotime() - last_sleep_times[p_list->proc_type];
		ptype_temp = p_list->proc_type;
		if(send_instruction(&p_list, str)) {
		// Process finished
			turn_around_times[ptype_temp] = nanotime() - arrival_times[ptype_temp];
			total--;
			continue;
		}
		
		//Round Robin algorithm
		if(sh == RR) {
			nanosleep(&ts, &ts);
			
			strcpy(str, "sleep");
			last_sleep_times[p_list->proc_type] = nanotime();
			ptype_temp = p_list->proc_type;
			if(send_instruction(&p_list, str)) {
				turn_around_times[ptype_temp] = nanotime() - arrival_times[ptype_temp];
				total--;
				continue;
			}
		}
		else { //First Come First Serve Algorithm
			while(!p_list->shseg->finished);
			
			strcpy(str, "finish");
			ptype_temp = p_list->proc_type;
			send_instruction(&p_list, str);
			turn_around_times[ptype_temp] = nanotime() - arrival_times[ptype_temp];
			total--;
			continue;
		}
		
		//next process in line for processing
		p_list = p_list->next;
	}
	
	//Accquire results from the processes
	int procs = 3;
	while(procs--) printf("Child destroyed: %d\n", wait(NULL));
	printf("All done\n");
	procs = 3;
	FILE* file = fopen("profile.txt", "a");
	fprintf(file, "%d, %d, %d, %s, ", atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), argv[4]);
	while(procs--) {
		//printf("Arrival time for C%c: %.3f\n", 2 - procs + '1', (float)arrival_times[2-procs]/1e6);
		printf("Watiting time for C%c: %.3f\n", 2 - procs + '1', (float)waiting_times[2-procs]/1e6);
		printf("TAT for C%c: %.3f\n", 2 - procs + '1', (float)turn_around_times[2-procs]/1e6);
		fprintf(file, "%.3f, %.3f, ", (double)waiting_times[2-procs]/1e6, (double)turn_around_times[2-procs]/1e6);
	}
	fprintf(file, "\n");
	
	return 0;
}
