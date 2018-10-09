#include"P3common.h"
#include<stdio.h>
#include<string.h>

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

void canonicalize(int *clkptr) {
	if(clkptr[1] > 1000000000) {
		clkptr[0] += clkptr[1] / 1000000000;
		clkptr[1] = clkptr[1] % 1000000000;
	}
}