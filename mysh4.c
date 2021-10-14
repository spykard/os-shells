#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char usrInput[256], *prog_argv[16][20];
	int i, y, z, cc, count, status, pid[2], fd[2], fd_in;
	pid_t pidM;

	//gethostname(hostn, sizeof(hostn));
	//user = (char *)getenv("USER"); // Get name of user instead of $ prompt

	while(1)
	{	printf("$"); // PROMPT

		// User Input
		if (!fgets(usrInput, sizeof(usrInput), stdin)) break;

		if (strncmp("exit", usrInput, 4) == 0)
		{	exit(0);} // QUIT
		else if( strncmp("cd", usrInput, 2) == 0)
		{	char *cdInput = strtok(usrInput, " \n"); // CD
			cdInput = strtok(NULL, " \n");
			//getcwd(cwd,sizeof(cwd));
			//strcat(cwd,"/"); 
			//strcat(cwd,cdInput);
			if (!cdInput)
				fprintf(stderr, "cd missing argument\n"); // Input validation and segmentation fault prevention
			else
				chdir(cdInput);
			continue;}		

		char *sInput = (char *)malloc((sizeof(usrInput)+1) * sizeof(char));
		char *pipeInput = (char *)malloc((sizeof(usrInput)+1) * sizeof(char));
		i=0;
		y=0;
		count=0;

		sInput = strtok(usrInput, " \n"); // Parse user input

	 	while ( sInput != NULL ) 
	 	{	prog_argv[0][i++] = sInput;
	 		sInput = strtok(NULL, " \n");}

	 	prog_argv[0][i] = NULL; // Null at end of array

	 	for ( i = 0; prog_argv[0][i] != NULL; i++)  // Count number of pipes on input
	 		if (strcmp(prog_argv[0][i], "|") == 0)
	 			count++;		 					// Number of piped commands
 
	 	for (z = 1; z <= count; z++)				// Similar to mysh3 but multiple pipes for multiple commands
	 	{	cc=0;
	 		y++;
		 	for (i = 0; prog_argv[y-1][i] != NULL; i++){ // Split commands at |
				if (!strcmp(prog_argv[y-1][i], "|")) {
					prog_argv[y-1][i] = NULL;
					while (prog_argv[y-1][i+1] != NULL) {
						prog_argv[y][cc] = prog_argv[y-1][i+1];
						prog_argv[y-1][i+1] = NULL;
						i++;
						cc++;}
					}}
			prog_argv[y][cc] = NULL;}

	prog_argv[y+1][0] = NULL;

		if (count == 1) // Input with 1 pipe
		{	if (pipe(fd)<0)
			{	perror("Pipe error");
	        	exit(98);}

		 	pid[0] = fork();	

		 	if (pid[0]<0) 
		 	{	perror("Fork error");
		 		exit(99);}


			if (!pid[0])
			{	close (fd[0]); // OUT
				dup2 (fd[1], 1);
				close (fd[1]);
				execvp(prog_argv[0][0], prog_argv[0]);
				//perror("Error calling exec()");
		 		exit(1);}

			if (pid[0]) // FATHER
			{	if ((pid[1]=fork())<0)
		 		{	perror("Fork error");
		 			exit(99);}

				if (!pid[1]) // CHILD
				{	close (fd[1]); // IN
					dup2 (fd[0], 0);
					close (fd[0]);
					execvp(prog_argv[1][0], prog_argv[1]);
					//perror("Error calling exec()");
					exit(1);}

				close(fd[0]);
				close(fd[1]);
				waitpid (pid[1], &status, 0);}}
				//exit(1);
		else if (count == 0) // Input without pipes
		{	pid[0] = fork();	
		 	
		 	if (pid[0]<0) 
		 	{	perror("Fork error");
		 		exit(99);}
		 	else if (pid[0]==0) // CHILD
		 	{	execvp(prog_argv[0][0], prog_argv[0]);
		 		//perror("Error calling exec()");
		 		exit(1);}
		 	else              	// FATHER
		 		waitpid (pid[0], &status, 0);}
		else    			  	// Input with multiple pipes
		{	int p[2];
			fd_in = 0;
			i=0;

			while (prog_argv[i][0] != NULL)
		    {	pipe(p);
		    	if ((pidM = fork()) == -1)
		        {	perror("Fork error");
		        	exit(99);}
				else if (pidM == 0)
		        {	dup2(fd_in, 0);  // Change based on old one
		         	if (prog_argv[i+1][0] != NULL)
		        		dup2(p[1], 1);
		        	close(p[0]);
		        	execvp(prog_argv[i][0], prog_argv[i]);
		        	//perror("Error calling exec()");
		        	exit(1);}
		      	else
		        {	waitpid (pidM, &status, 0);
		        	close(p[1]);
		        	fd_in = p[0];    // Save new one
		        	i++;}}}
	}	
	return 0;
}