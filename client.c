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
#include <time.h>
#include <semaphore.h>
#include "buffer.h"

//assume the range of page of a file is 1 to 10
#define MIN_PAGES	1
#define MAX_PAGES	10

const char *NAME = "/shm-printer";

Buffer * shared_mem_ptr;
int shm_fd;

int attach_shared_mem();
void produce_a_job();
int generate_random_number();

/*
 * This program implements client code to generate print jobs
 */
int main(int argc, char *args[]) {

	if (argc <= 1) {
		printf("Enter the filename to print\n");
		exit(0);
	}

	attach_shared_mem();
	//create print job and add it
	produce_a_job(args[1]);

	return 1;
}

/*
 * Connect the client to the same shared memory segment
 */
int attach_shared_mem(void) {

	//open the shared memory segment as if it was a file
	shm_fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1) {
		printf("Shared memory failed: %s\n", strerror(errno));
		exit(1);
	}

	//configure the size of the shared memory segment
	ftruncate(shm_fd, sizeof(Buffer));

	//map the shared memory segment to the address space of the process
	shared_mem_ptr = (Buffer *) mmap(
		0,
		sizeof(Buffer),
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

void produce_a_job(char *filename) {

	//the server is not opened yet
	if (shared_mem_ptr->size <= 0)
	{
		printf("error: Need to run server first.\n");
		exit(0);
	}

	//If the buffer is full, no print job can be put on the buffer
	//otherwise, client can create a job which can be successfully put on the buffer
	if (shared_mem_ptr->request == shared_mem_ptr->size) {
		printf("Buffer is full. Please Wait\n");
	}
	else {
		//increment the counter for print request
		shared_mem_ptr->request++;

		//create a job including job name, client ID, and pages number of the job
		//both the client ID and the pages number are generated randomly
		//client has ID number from 0 to 20 and the range of page number is [1,10]
		Job new_task;
		strcpy(new_task.name, filename);
		new_task.pages_num = generate_random_number(MIN_PAGES, MAX_PAGES);
		new_task.client_ID = generate_random_number(0, 20);

		//print a message of client's file
		if (new_task.pages_num == 1) {
			printf("Client %d has %d page to print, puts request in buffer\n",
				new_task.client_ID,
				new_task.pages_num);
		}
		else {
			printf("Client %d has %d pages to print, puts request in buffer\n",
				new_task.client_ID,
				new_task.pages_num);
		}

		//wait until cleint gets to access the buffer
		sem_wait(&shared_mem_ptr->underflow);
		sem_wait(&shared_mem_ptr->mutex);

		//put the job on the buffer
		shared_mem_ptr->jobs[shared_mem_ptr->out] = new_task;

		shared_mem_ptr->out = (shared_mem_ptr->out + 1) % shared_mem_ptr->size;

		//let others use the buffer
		sem_post(&shared_mem_ptr->mutex);
		sem_post(&shared_mem_ptr->overflow);

		shmdt(shared_mem_ptr);

		printf("Job is created and added successfully.\n");
	}

}

/*
 * Generate random value
 */
int generate_random_number(int minimum, int maximum) {

	srand(time(NULL));
	return (minimum + rand() % maximum);
}