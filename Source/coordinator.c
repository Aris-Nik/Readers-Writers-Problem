#include "coordinator.h"


int main(int argc, char* argv[]){

	srand(time(NULL));
	int peers = atoi(argv[1]);
	int entries = atoi(argv[2]);
	double ratio = atof(argv[3]);		// ratio = readers/writers (approximately)
	int iterations = atoi(argv[4]);
	float w=0.0;
	int writers=0;
	int readers=0;
	printf("Peers = %d entries = %d ratio = %f iterations = %d \n",peers,entries,ratio,iterations);
	if (iterations == 1 && ratio < 0.5)			//Case where we can have 0 readers but 1 writer
		writers = 1;
	else{
		w = (float)iterations/(float)(ratio + 1);		//Number of writers
		writers = iterations/(ratio + 1);
		if (fabs(w-writers) > 0.5){
			writers++;
		}

		readers = iterations-writers;			//Number of readers
	}

	printf("Writers = %d  Readers = %d \n\n",writers,readers);

	int shmid;
	Entry* shmPTR;
	shmid = shmget(IPC_PRIVATE,sizeof(entries * sizeof(Entry)),0644|IPC_CREAT);		//Allocate shared memory
	
	if (shmid == -1){
		printf("Error on shared memory \n");
		return 1;
	}

	shmPTR = (Entry *) shmat(shmid, NULL, 0);

	for (int i = 0; i < entries ; i++){
		sem_init(&(shmPTR[i].semWriter),1,1);
		sem_init(&(shmPTR[i].semReader),1,1);
	}


	for (int i = 0; i < entries; i++){
		shmPTR[i].writes = 0;
		shmPTR[i].reads = 0;
		shmPTR[i].readcount = 0;
	}
	pid_t pid;

	for (int i = 0; i < peers; i++){				//Create n child processes
		int average_time = 0;
		pid = fork();
		if (pid == 0)								//If child process then break and dont fork
			break;
	}

	if (pid == 0){
		int countW = 0;								//counter for how many writes we do on every iteration
		int countR = 0;								//counter for how many reads we do on every iteration
		double average_time = 0.0;
		struct timeval start;
		struct timeval end;
		double total = 0.0;
		for (int i = 0; i < iterations; i++){
			srand(time(NULL) * getpid());			//seed for rand (multiply by a different number which is getpid() every different child so it does not generate the same number)
			int random = rand()  % entries ;
			int operation = rand() % 2;			// 50% chance to perform read or write 														//choose operation = 1 for write and operation = 0 for read
			if (operation == 1 && countW < writers || operation == 0 && countR == readers){	//if operation is write and we still have some writers then write OR if operation is read and we cant use any more readers then write again			//if operation is write and we have peers left to write then use write OR if operation is read but we have used all peers for read then use write
				countW++;
				gettimeofday(&start,NULL);								//algorithm from GeeksforGeeks that counts seconds
				sem_wait(&(shmPTR[random].semWriter));
				gettimeofday(&end,NULL);								//algorithm from GeeksforGeeks that counts seconds
				total = (end.tv_sec - start.tv_sec) * 1e6;				//algorithm from GeeksforGeeks that counts seconds
				total = (total + (end.tv_usec - start.tv_usec)) * 1e-6;	//algorithm from GeeksforGeeks that counts seconds
				average_time += total;									//algorithm from GeeksforGeeks that counts seconds
				shmPTR[random].writes = shmPTR[random].writes + 1;
				sleep(Exponential(0.7));
				sem_post(&(shmPTR[random].semWriter));
			}
			else{
				countR++;
				gettimeofday(&start,NULL);
				sem_wait(&(shmPTR[random].semReader));
				gettimeofday(&end,NULL);

				total = (end.tv_sec - start.tv_sec) * 1e6;	
				total = (total + (end.tv_usec - start.tv_usec)) * 1e-6;
				average_time += total;

				shmPTR[random].readcount = shmPTR[random].readcount + 1 ;

				if (shmPTR[random].readcount == 1){						//If first reader
					sem_wait(&(shmPTR[random].semWriter));
				}

				sem_post(&(shmPTR[random].semReader));
				
				shmPTR[random].reads = shmPTR[random].reads + 1;
				sleep(Exponential(0.7));

				gettimeofday(&start,NULL);
				sem_wait(&(shmPTR[random].semReader));
				gettimeofday(&end,NULL);

				total = (end.tv_sec - start.tv_sec) * 1e6;	
				total = (total + (end.tv_usec - start.tv_usec)) * 1e-6;
				average_time += total;

				shmPTR[random].readcount = shmPTR[random].readcount - 1 ;

				if (shmPTR[random].readcount == 0)					//If "last" reader
					sem_post(&(shmPTR[random].semWriter));

				total = (end.tv_sec - start.tv_sec) * 1e6;
				total = (total + (end.tv_usec - start.tv_usec)) * 1e-6;
				average_time += total;

				sleep(Exponential(0.7));

				sem_post(&(shmPTR[random].semReader));
			}
		}
		average_time = average_time / (double) iterations;
		printf("Process with parent id %d writes %d and reads %d with average waiting time : %.7f seconds\n",getpid(),countW,countR,average_time);
		exit(0);						//child exits
	}
	int status = 0;
    while(wait(&status) > 0);  // waiting 

    for (int i = 0; i < entries; i++){
		printf("Entry %d has %d writes and %d reads \n",i+1,shmPTR[i].writes,shmPTR[i].reads);
	}
	for (int i = 0; i < entries ; i++){							//Destroying semaphores
		if (sem_destroy(&shmPTR[i].semWriter) == -1 || sem_destroy(&shmPTR[i].semReader) == -1 )
			printf("*Semaphore deletion failed*\n");

	}
	if (shmdt(shmPTR) == -1) {                 					//Detaching SharedMemory
        printf(" *shared memory detach failed*\n");
        exit(1);
    }
    if (shmctl(shmid, IPC_RMID, 0) == -1) {						//Deleting SharedMemory
        printf(" *shmctl(IPC_RMID) failed*\n"); 
        exit(1);
    }
} 

double Exponential (double l){

	double u;
	u = rand() / (RAND_MAX + 1.0);
	double x = -log(1 - u) / l;
	return x;
}