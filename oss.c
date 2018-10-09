#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<signal.h>
#include<string.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include"P3common.h"



void killAllChildren();
void handle_alarm();
void handle_intr();
void increment(int *, int );

int childCount = 0,
	j = 0,
	maxUserProc = 5,
	mstrTermTime = 2,
	*shared;
pid_t *childpid;

int main (int argc, char *argv[]) {

	int c,
		*clock,
		i,
		index,
		semID,
		shmID,
		*shmMsg,
		waitint;
	char *ossLogFileName = "ossLog.txt",
		errPreString[120],
		childArgK[60],
		childArgS[60];
	FILE *logfile;
	key_t shmKey;
	size_t shmSize;
	
	srand(time(0));

	//Command line argument processing
	opterr = 0;
	while ((c = getopt (argc, argv, "hl:s:t:")) != -1) {
	switch (c)
	{
	case 'h':
		helpMsgFunction(argv[0]);
		return 0;
	case 's':
		maxUserProc = atoi(optarg);
		break;
	case 't':
		mstrTermTime = atoi(optarg);
		break;
	case 'l':
		ossLogFileName = optarg;
		break;
	case '?':
		if (optopt == 't' || optopt == 's')
			fprintf (stderr, "Option -%c requires an integer argument.\n", optopt);
		else if (optopt == 'l')
			fprintf (stderr, "Option -%c requires a string argument.\n", optopt);
		else if (isprint (optopt))
			fprintf (stderr, "Unknown option `-%c'.\n", optopt);
		else
			fprintf (stderr,
			"Unknown option character `\\x%x'.\n",
			optopt);
		return 1;
	default:
		abort ();
	}
	}
	for (index = optind; index < argc; index++)
		printf ("Non-option argument %s\n", argv[index]);

	
	//Signal Handling:
	signal(SIGINT, handle_intr);
	signal(SIGALRM, handle_alarm);	
	alarm(mstrTermTime);
	
	//Allocate storage of child pids:
	childpid = (pid_t *)malloc(maxUserProc * sizeof(pid_t));
	
	//Open the child log file
	logfile = fopen(ossLogFileName, "w+");
	
	//Shared memory setup:
	if( ( shmKey = ftok(ossLogFileName, rand()%255 + 1) ) == -1 ) {
		errMsgFunction(errPreString, argv[0], "Cannot create shared memory key");
		return -1;
	}
	shmSize = 2*sizeof(int) + 81*sizeof(char);
	if( ( shmID = shmget(shmKey, shmSize, IPC_CREAT|IPC_EXCL|0640) ) == -1 ) {
		errMsgFunction(errPreString, argv[0], "Parent cannot create shared memory segment");
		return -1;
	}
	if( ( shared = (int *)shmat(shmID, 0, 0) ) == (void *)-1 ) {
		errMsgFunction(errPreString, argv[0], "Parent cannot attach to shared memory");
		return -1;
	}
	if(( semID = semget(shmKey, 1, IPC_CREAT | IPC_EXCL | 0640) ) == -1) {
		errMsgFunction(errPreString, argv[0], "Semaphore creation failed");
	}
	
	//Initialize the shared clock and shared message area.
	//The message will consist of three integers: the child's PID,
	//and the two integers of the clock reading at that child's
	//message time.
	clock = shared;
	clock[0] = clock[1] = 0;
	shmMsg = (shared + 2);
	shmMsg[0] = shmMsg[1] = shmMsg[2] = 0;
	
	//fork/exec
	do {
		while(childCount < maxUserProc) {
			switch ( childpid[j] = fork() )
			{
			case -1:
				errMsgFunction(errPreString, argv[0], "Fork failure");
				return -1;
			case 0:	//child
				sprintf(childArgK, "%59d", shmKey);
				sprintf(childArgS, "%59d", shmSize);
				if( execl("./user", "./user", childArgK, childArgS, (char *)NULL) == -1) {
					errMsgFunction(errPreString, argv[0], "Exec failure");
					return -1;
				}
				break;
			default:	//parent
				increment(clock, 333333);
				j += 1;
				j = j % maxUserProc;
				childCount++;
				break;
			}
		}
		wait(&waitint);
		childCount--;
	} while(childCount > 0);
	
	
	//Handle normal termination
	if( shmdt(shared) == -1 ) {
		errMsgFunction(errPreString, argv[0], "Parent failed to detatch shared memory");
		return -1;
	}
	if( shmctl(shmID, IPC_RMID, 0) == -1 ) {
		errMsgFunction(errPreString, argv[0], "Failed to free shared memory segment");
		return -1;
	}
	fclose(logfile);
	return 0;
}



void killAllChildren() {
	int i;
	for (i = 0; i < maxUserProc; i++) {
		if(kill(childpid[i], SIGTERM) == -1)
			perror("Failed to kill a child");
		
	}
}

void handle_alarm() {
	perror("Timer expired");
	killAllChildren();
	exit(-1);
}

void handle_intr() {
	perror("Received Ctrl+C");
	killAllChildren();
	exit(-1);
}

void increment(int *clkptr, int inc) {
	clkptr[1] += inc;
	canonicalize(clkptr);
}
