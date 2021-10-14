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
	int i, sema, semb;
	struct sembuf up = {0, 1, 0}; // 2o parameter of semop
	struct sembuf down = {0, -1, 0}; // 2o parameter of semop

	sema = semget(IPC_PRIVATE, 1, 0600); // create
	semb = semget(IPC_PRIVATE, 1, 0600); // create

	semop(semb, &up, 1); // up so it can initially go into "ab"

	if (fork())
	{
		for (i=0;i<10;i++){
			semop(semb, &down, 1); // down		
			display("ab");
			semop(sema, &up, 1); // up	
		}
		wait(NULL);
	}
	else
	{
		for (i=0;i<10;i++){
			semop(sema, &down, 1); // down		
			display("cd\n");
			semop(semb, &up, 1); // up		
		}
	}

	semctl(sema, 0, IPC_RMID); // destroy
	semctl(semb, 0, IPC_RMID); // destroy	
	return 0;
}
