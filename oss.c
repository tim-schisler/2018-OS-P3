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
void semLog(char *, int *, int *);
void freeShared(int *, int , int , char *, char *);

FILE *logfile;
char errPreString[120];
int childCount = 0,
	j = 0,
	maxUserProc = 5,
	mstrTermTime = 2,
	semID,
	shmID,
	*shared;
pid_t *childpid;

int main (int argc, char *argv[]) {

	int c,
		*clock,
		i,
		index,
		*shmMsg,
		waitint;
	char *ossLogFileName = "ossLog.txt",
		childArgK[60],
		childArgS[60];
	
	key_t shmKey;
	pid_t killed;
	size_t shmSize;
	struct sembuf waitOp = {0, -1, 0},
			signalOp = {0, 1, 0};
	union semun arg;
	
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
	arg.val = 1;
	if( semctl(semID, 0, SETVAL, arg) == -1 ) {
		errMsgFunction(errPreString, argv[0], "Failed to initialize semaphore");
		return -1;
	}
	
	//Initialize the shared clock and shared message area.
	clock = shared;
	clock[0] = clock[1] = 0;
	shmMsg = (shared + 2);
	shmMsg[0] = shmMsg[1] = shmMsg[2] = 0;

	//Prepare arguments to pass to children processes
	sprintf(childArgK, "%59d", shmKey);
	sprintf(childArgS, "%59d", shmSize);
	
	//Fork the first generation of children
	for(j = 0; j < maxUserProc; j++, childCount++) {
		switch ( childpid[j] = fork() )
		{
		case -1:
			errMsgFunction(errPreString, argv[0], "Fork failure");
			return -1;
		case 0:	//child
			if( execl("./user", "./user", childArgK, childArgS, (char *)NULL) == -1) {
				errMsgFunction(errPreString, argv[0], "execl() failure");
				return -1;
			}
			break;
		default:	//parent
			break;
		}
	}
	
	//Simulation loop:
	do {
		increment(clock, 3333);
		if(semop(semID, &waitOp, 1) == -1) {
			errMsgFunction(errPreString, argv[0], "semaphore wait failed");
			return -1;
		}
		if(shmMsg[0] != 0) {
			semLog(argv[0], shmMsg, clock);
			if((killed = waitpid((pid_t)shmMsg[0], &waitint, 0))==(pid_t)-1) {
				errMsgFunction(errPreString, argv[0], "waitpid() failure");
				return -1;
			}
			shmMsg[0] = shmMsg[1] = shmMsg[2] = 0;
			//Search the array for the position of the old child;
			//spawn a new child process in its place.
			for(j = 0; j < maxUserProc; j++) {
				if(childpid[j] == killed) {
					switch ( childpid[j] = fork() )
					{
					case -1:
						errMsgFunction(errPreString, argv[0], "Fork failure");
						return -1;
					case 0:	//child
						if( execl("./user", "./user", childArgK, childArgS, (char *)NULL) == -1) {
							errMsgFunction(errPreString, argv[0], "execl() failure");
							return -1;
						}
						break;
					default:	//parent
						break;
					}
					break;
				}
			}
			childCount++;
		}
		if(semop(semID, &signalOp, 1) == -1) {
			errMsgFunction(errPreString, argv[0], "semaphore signal failed");
			return -1;
		}
	} while( 1 );	//(childCount <= 100) && (clock[0] < 3)
	
	
	//Handle normal termination
	freeShared(shared, shmID, semID, errPreString, argv[0]);
	fclose(logfile);
	free(childpid);

	return 0;
}



void killAllChildren() {
	int i;
	for (i = 0; i < maxUserProc; i++)
		if(kill(childpid[i], SIGTERM) == -1)
			perror("Failed to kill a child");
}

void handle_alarm() {
	perror("Timer expired");
	killAllChildren();
	freeShared(shared, shmID, semID, errPreString, "Alarm handler: ");
	exit(-1);
}

void handle_intr() {
	perror("Received Ctrl+C");
	killAllChildren();
	freeShared(shared, shmID, semID, errPreString, "Interrupt handler: ");
	exit(-1);
}

void increment(int *clkptr, int inc) {
	clkptr[1] += inc;
	canonicalize(clkptr);
}

void semLog(char *arg, int *msg, int *clk) {
	fprintf(logfile, "%s: Child %d reached %d.%-09.9d in user process. Terminating at OSS time %d.%-09.9d\n", arg, msg[0], msg[1], msg[2], clk[0], clk[1]);
}

void freeShared(int *shm, int shmid, int semid, char *err, char *arg) {
	if( shmdt(shm) == -1 ) {
		errMsgFunction(err, arg, "Parent failed to detatch shared memory");
		exit(-1);
	}
	if( shmctl(shmid, IPC_RMID, 0) == -1 ) {
		errMsgFunction(err, arg, "Failed to free shared memory segment");
		exit(-1);
	}
	if( semctl(semid, 0, IPC_RMID) == -1 ) {
		errMsgFunction(err, arg, "failed to remove semaphore");
		exit(-1);
	}
}