# Printer Spooler

## DESCRIPTION

All source code are provided in the the zip folder.

There are 3 assumptions made for this assignment:
	1. Only 1 printer server
	2. Printer takes 1 second per page to complete the task
	3. Muliple clients have access to server

A trace example of this program is shown in trace_example.pdf


## COMPILATION INSTRUCTIONS

Compile the source code using these commands:

gcc server.c -lrt -lpthread -o server.out
gcc client.c -lrt -lpthread -o client.out


## COMMAND LINE ARGUMENTS

- Run "server.out" with the following:
	./server.out BUFFER_SIZE
	(required) Specifies the buffer size for the shread memory segment
         which is determined by the user

	BUFFER_SIZE - positive integer
	If the input is negative value or zero, the buffer cannot be created           and the server turns off


- Run "client.out" with the following:
	./client.out FILENAME
	(required) Specifies the print filename
	 If the filename is missing, client needs to re-send the print job
	 Cannot be run before the server; otherwise, client cannot send any             print job to the server
