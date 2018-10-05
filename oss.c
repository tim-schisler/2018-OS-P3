#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<signal.h>
#include<string.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>


void helpMsgFunction(char *);
void errMsgFunction(char * , char * , char * );
void killAllChildren();
void handle_alarm();
void handle_intr();

int childCount = 0,
	j = 0,
	maxUserProc = 5,
	mstrTermTime = 2,
	*shared;
pid_t *childpid;

int main (int argc, char *argv[]) {

	int c,
		i,
		index,
		shmID,
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
	
	//Shared Memory:
	if( ( shmKey = ftok(ossLogFileName, rand()%255 + 1) ) == -1 ) {
		errMsgFunction(errPreString, argv[0], "Cannot create shared memory key");
		return -1;
	}
	shmSize = 2*sizeof(int);
	if( ( shmID = shmget(shmKey, shmSize, IPC_CREAT|IPC_EXCL|0640) ) == -1 ) {
		errMsgFunction(errPreString, argv[0], "Parent cannot create shared memory segment");
		return -1;
	}
	if( ( shared = (int *)shmat(shmID, 0, 0) ) == (void *)-1 ) {
		errMsgFunction(errPreString, argv[0], "Parent cannot attach to shared memory");
		return -1;
	}
	
	
	//fork/exec
	do {
		while(childCount < maxUserProc) {
			switch ( childpid[j] = fork() )
			{
			case -1:
				errMsgFunction(errPreString, argv[0], "Fork failure");
				return -1;
			case 0:	//child
				sprintf(childArgK, "%d", shmKey);
				sprintf(childArgS, "%d", shmSize);
				if( execl("./user", "./user", childArgK, childArgS, (char *)NULL) == -1) {
					errMsgFunction(errPreString, argv[0], "Exec failure");
					return -1;
				}
				break;
			default:	//parent
				++j % maxUserProc;
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
	for(i = 0; i < j; i++) {
		free(&childpid[i]);
	}
	fclose(logfile);
	return 0;
}



void helpMsgFunction(char *enm) {
	
	printf("\nInvoke %s with the following options:\n", enm);
	printf("\t-h to print this help message.\n");
	printf("\t-s x, where x is an integer.\n");
	printf("\t\ts specifies how many children to create.\n");
	printf("\t-t z, where z is an integer.\n");
	printf("\t\tt specifies how many seconds %s may run.\n", enm); 
	printf("\t-l filename\n");
	printf("\t\tl specifies the location of the log file.\n");
}

void errMsgFunction(char *s, char *arg, char *func) {
	strcpy(s, arg);
	strcat(s, func);
	perror(s);
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

