#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>



void helpMsgFunction(char *);

int main (int argc, char *argv[]) {

	int c,
		index,
		maxUserProc = 5,
		mstrTermTime = 2;

	//Command line argument processing
	opterr = 0;
	while ((c = getopt (argc, argv, "hl:s:t:")) != -1)
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
		break;
	case '?':
		if (optopt == 't' || optopt == 's')
			fprintf (stderr, "Option -%c requires an integer argument.\n", optopt);
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
	for (index = optind; index < argc; index++)
		printf ("Non-option argument %s\n", argv[index]);

	
	
	

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
