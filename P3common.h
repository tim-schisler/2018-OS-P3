#ifndef P3COMMON_H
#define P3COMMON_H

union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
							   (Linux-specific) */
};
	
void helpMsgFunction(char *);
void errMsgFunction(char * , char * , char * );
void canonicalize(int *);

#endif