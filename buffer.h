/*
ECSE 427 mini-Assignment 2 Printer Spooler
Author: Wei Wang, wei.wang18@mail.mcgill.ca
Student ID: 260580783
Date: 2015-10-19
*/

#ifndef BUFFER_H_
#define BUFFER_H_

/*
 * Job
 */
typedef struct
{
	char name[20];		/*filename, at most 20 characters*/
	int client_ID;		/*Client ID of the file*/
	int pages_num;		/*total page number of hte file*/
}Job;

/*
 * Buffer
 */
typedef struct
{
	int size;			/*buffer's size, decided by user*/
	int in;				/*first empty slot*/
	int out;			/*first full slot*/
	int request;		/*counter for entered jobs*/
	sem_t overflow;		/*keep track of the number of full spots*/
	sem_t underflow;	/*keep track of the number of empty spots*/
	sem_t mutex;
	Job jobs[1];		/*shared data*/
}Buffer;

#endif /* BUFFER_H_ */