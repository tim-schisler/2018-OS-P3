#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<limits.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include"P3common.h"

#define DEBUG 0

int main (int argc, char *argv[]) {

	char errPreString[120];
	int c,
		*clock,
		deadline[2],
		index,
		lifespan,
		*shared,
		semID,
		shmID,
		shmKey,
		*shmMsg,
		shmSize;
	
	struct sembuf waitOp = {0, -1, 0},
			signalOp = {0, 1, 0};
	
	srand(time(0));
	
	//Command line argument processing
	if(argc != 3) {
		fprintf(stderr, "Child %ld was passed incomplete args.\n", (long)getpid());
		return -1;
	} else {
		shmKey = atoi(argv[1]);
		shmSize = atoi(argv[2]);
	}
	
	//Set Up Shared Memory
	if( ( shmID = shmget(shmKey, shmSize, 0) ) == -1 ) {
		errMsgFunction(errPreString, argv[0], "cannot identify shared memory segment");
		return -1;
	}
	if( ( shared = (int *)(shmat(shmID, 0, 0)) ) == (void *)-1 ) {
		errMsgFunction(errPreString, argv[0], "cannot attach to shared memory");
		return -1;
	}
	clock = shared;
	shmMsg = (shared + 2);
	if(( semID = semget(shmKey, 1, 0640) ) == -1) {
		errMsgFunction(errPreString, argv[0], "failed to find semaphore");
	}
	
	//Generate the termination deadline
	lifespan = rand()%1000000;
	deadline[0] = clock[0] + lifespan%1000000000;
	deadline[1] = lifespan/1000000000;
	canonicalize(deadline);
	
	//Critical region:
	do {
		if(semop(semID, &waitOp, 1) == -1) {
			errMsgFunction(errPreString, argv[0], "semaphore wait failed");
			return -1;
		}
		if((clock[0] >= deadline[0]) || (clock[1] >= deadline[1])) {
			if(shmMsg[0] = 0) {
				shmMsg[0] = getpid();
				shmMsg[1] = clock[0];
				shmMsg[2] = clock[1];
				if(semop(semID, &signalOp, 1) == -1) {
					errMsgFunction(errPreString, argv[0], "semaphore signal failed");
					return -1;
				}
				break;
			}
		}
		if(semop(semID, &signalOp, 1) == -1) {
			errMsgFunction(errPreString, argv[0], "semaphore signal failed");
			return -1;
		}
	} while(1);
	

	//printf("Child %ld reporting. Clock: %d.%-09.9dsec\n", (long)getpid(), clock[0], clock[1]);
	
	//Detatch from Shared Memory
	if( shmdt(shared) == -1 ) {
		errMsgFunction(errPreString, argv[0], "failed to detatch shared memory");
		return -1;
	}
	
	return 0;
}

