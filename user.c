#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int main (int argc, char *argv[]) {

	printf("Child %ld reporting.\n", (long)getpid());

	return 0;
}

