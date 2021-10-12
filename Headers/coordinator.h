#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include <sys/time.h>
#include <sys/wait.h> 
#include <math.h>
#include <semaphore.h>

typedef struct Entry{
	int writes;					//total writes on that entry
	int reads;					//total reads on that entry
	int readcount;				//current readers using that entry
	sem_t semWriter;			//semaphore for writers
	sem_t semReader;			//semaphore for readers
}Entry;

double Exponential (double l);