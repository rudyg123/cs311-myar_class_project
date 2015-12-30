/* Rudy Gonzalez
   gonzalru@onid.orst.edu
   CS311-400
   Homework 4
*/

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void sigHandler_USR1(int sig)
{
	printf("SIGUSR1 has been caught\n");
}

void sigHandler_USR2(int sig)
{
	printf("SIGUSR2 has been caught\n");
}

void sigHandler_INT(int sig)
{
	printf("SIGINT has been caught, terminating the program\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	if (signal(SIGUSR1, sigHandler_USR1) == SIG_ERR)
	{
		printf("signal error has occurred\n");
		exit(1);
	}
	kill(getpid(), SIGUSR1);
    
	if (signal(SIGUSR2, sigHandler_USR2) == SIG_ERR)
	{
		printf("signal error has occurred\n");
		exit(1);
	}
	kill(getpid(), SIGUSR2);
	
	if (signal(SIGINT, sigHandler_INT) == SIG_ERR)
	{
		printf("signal error has occurred\n");
		exit(1);
	}
	kill(getpid(), SIGINT);

	return 0;
}


