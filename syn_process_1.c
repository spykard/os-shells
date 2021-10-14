#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "display.h"

#include <sys/ipc.h>
#include <sys/sem.h>

int main()
{
	int i, sema;
	struct sembuf up = {0, 1, 0}; // 2o parameter of semop
	struct sembuf down = {0, -1, 0}; // 2o parameter of semop

	sema = semget(IPC_PRIVATE, 1, 0600); // create

	if (fork())
	{
		semop(sema, &down, 1); // down
		for (i=0;i<10;i++)
			display("Hello world\n");
		wait(NULL);
	}
	else
	{
		for (i=0;i<10;i++)
			display("Kalimera kosme\n");
		semop(sema, &up, 1); // up	
	}

	semctl(sema, 0, IPC_RMID); // destroy
	return 0;
}
