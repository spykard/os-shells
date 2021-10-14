#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char usrInput[256], *prog_argv[64];
	int i, status;
	pid_t sxpid;

	//gethostname(hostn, sizeof(hostn));
	//user = (char *)getenv("USER"); // Get name of user instead of $ prompt

	while(1)
	{	printf("$"); // PROMPT

		// User Input
		if (!fgets(usrInput, sizeof(usrInput), stdin)) break;

		if (strncmp("exit", usrInput, 4) == 0)
		{	exit(0);} // QUIT
		else if (strncmp("cd", usrInput, 2) == 0)
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
		i=0;

		sInput = strtok(usrInput, " \n"); // Parse user input

	 	while ( sInput != NULL ) 
	 	{	prog_argv[i++] = sInput;
	 		sInput = strtok(NULL, " \n");}

	 	prog_argv[i] = NULL; // Null at end of array

	 	sxpid = fork();	
	 	
	 	if (sxpid<0) 
	 	{	perror("Fork error");
	 		exit(99);}
	 	else if (sxpid==0) // CHILD
	 	{	execvp(prog_argv[0], prog_argv);
	 		//perror("Error calling exec()");
	 		exit(1);}
	 	else               // FATHER
	 		waitpid (sxpid, &status, 0);
	}
	return 0;
}