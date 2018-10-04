#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int main (int argc, char *argv[]) {

	int *shared,
		shmID = atoi(argv[1]);

	if( ( shared = (int *)shmat(shmID, 0, 0) ) == (void *)-1 ) {
		perror("Child cannot attach to shared memory");
		return -1;
	}
	
	
	sleep(1);
	printf("Child %ld reporting.\n", (long)getpid());
	
	
	if( shmdt(shared) == -1 ) {
		perror("Child failed to detatch shared memory");
		return -1;
	}
	
	return 0;
}

