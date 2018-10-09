#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>

#define DEBUG 0

int main (int argc, char *argv[]) {

	int c,
		*clock,
		index,
		*shared,
		shmID,
		shmKey,
		shmSize;
	char *shmMsg;
	
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
		perror("Child cannot identify shared memory segment");
		return -1;
	}
	if( ( shared = /**/(int *)(shmat(shmID, 0, 0)) ) == (void *)-1 ) {
		perror("Child cannot attach to shared memory");
		return -1;
	}
	
	clock = shared;
	shmMsg = (char *)(shared + 2);
	
	printf("Child %ld reporting. Clock: %d.%-09.9dsec\n", (long)getpid(), clock[0], clock[1]);
	
	while(DEBUG);
	
	
	//Detatch from Shared Memory
	if( shmdt(shared) == -1 ) {
		perror("Child failed to detatch shared memory");
		return -1;
	}
	
	return 0;
}

