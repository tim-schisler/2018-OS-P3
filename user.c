#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int main (int argc, char *argv[]) {

	int c,
		index,
		*shared,
		shmID,
		shmKey,
		shmSize;
	
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
	if( ( shared = (int *)(shmat(shmID, 0, 0)) ) == (void *)-1 ) {
		perror("Child cannot attach to shared memory");
		return -1;
	}
	
	sleep(1);
	printf("Child %ld reporting.\n", (long)getpid());
	
	
	//Detatch from Shared Memory
	if( shmdt(shared) == -1 ) {
		perror("Child failed to detatch shared memory");
		return -1;
	}
	
	return 0;
}

