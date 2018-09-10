/*
ECSE 427 mini-Assignment 2 Printer Spooler
Author: Wei Wang, wei.wang18@mail.mcgill.ca
Student ID: 260580783
Date: 2015-10-19

References: semaphores_shm_tutorial from ECSE-427 tutorial by Syed Mushtaq Ahmed
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>
#include "buffer.h"

const char *NAME = "/shm-printer";

Buffer * shared_mem_ptr;
int shm_fd = 0;

int setup_shared_mem();
void terminate();

/*
 * This program implements printer code to print jobs in buffer
 */
int main(int argc, char *args[]) {

	//the command is not entered properly
	if (argc <= 1){
		printf("Restart the server with a buffer size\n");
		exit(0);
	}

	//Ctrl+C to terminate
	signal(SIGINT, terminate);

	//create a buffer using input size
	int size = strtol(args[1],NULL,10);

	if (size <= 0)
	{
		printf("Buffer size is not big enough. Please rerun the program with a positive buffer size input.\n");
		return 0;
	}

	setup_shared_mem(size);

	/*
 	 * Initialize the semaphore and buffer parameters.
 	 */
	shared_mem_ptr->in = 0;
	shared_mem_ptr->out = 0;
	shared_mem_ptr->request = 0;
	shared_mem_ptr->size = size;
	sem_init(&shared_mem_ptr->overflow, 1, 0);
	sem_init(&shared_mem_ptr->underflow, 1, size);
	sem_init(&shared_mem_ptr->mutex, 1, 1);

	//start to run the printer server
	printf("--------------- Starting the printer ---------------\n");
	while (1)
	{
		//wait for turn to access buffer, and verify whether the buffer is full
		sem_wait(&shared_mem_ptr->overflow);
		sem_wait(&shared_mem_ptr->mutex);

		/*
 	 	 * Take the job from buffer and remove it
 	 	 */
		Job task = shared_mem_ptr->jobs[shared_mem_ptr->in];
		shared_mem_ptr->request--;

		/*
		 * Print a message with file information
		 */
		if (task.pages_num == 1) {
			printf("Printer starts printing the job from Client %d: [%s] with %d page\n",
				task.client_ID,
				task.name,
				task.pages_num
				);
		}
		else {
			printf("Printer starts printing the job from Client %d: [%s] with %d pages\n",
				task.client_ID,
				task.name,
				task.pages_num
				);
		}

		shared_mem_ptr->in = (shared_mem_ptr->in + 1) % shared_mem_ptr->size;

		//let other clients to use the buffer
		sem_post(&shared_mem_ptr->mutex);
		sem_post(&shared_mem_ptr->underflow);

		/*
		 * Go sleep, by assuming that printer wait 1 second per page
		 */
		sleep(task.pages_num);

		//printer sleeps if there is not request in the buffer
		//until client send a signal to wake up
		int request = shared_mem_ptr->request;
		if (request == 0) {
			printf("No request in buffer, Pinter sleeps\n");
		}
	}

	return 0;
}
/*
 * Create and attach the shared memory segment
 */
int setup_shared_mem(int size) {

	//open the shared memory segment as if it was a file
	shm_fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1) {
		printf("Shared memory failed: %s\n", strerror(errno));
		exit(1);
	}

	//configure the size of the shared memory segment
	ftruncate(shm_fd, sizeof(Buffer) + size * sizeof(Job));

	//map the shared memory segment to the address space of the process
	shared_mem_ptr = (Buffer *) mmap(
		0,
		sizeof(Buffer) + size * sizeof(Job),
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		shm_fd,
		0
		);

	if (shared_mem_ptr == MAP_FAILED) {
		printf("error: Map failed: %s\n", strerror(errno));
		exit(1);
	}
}

void terminate(int sig) {

	//clean the shared memory segment before terminating the server and exit the program
	memset(shared_mem_ptr, 0, sizeof(Buffer) + shared_mem_ptr->size * sizeof(Job));
	close(shm_fd);
	munmap(shared_mem_ptr, sizeof(Buffer) + shared_mem_ptr->size * sizeof(Job));
	shm_unlink(NAME);

	printf("\nTerminating the server\n");
	exit(0);
}