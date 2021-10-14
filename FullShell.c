/*Writers: Matthias Braunhofer  matthias.braunhofer@stud-inf.unibz.it
           Manfred Ramoser      manfred.ramoser@stud-inf.unibz.it
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define TRUE 1
#define MAX 100

char *argv[MAX/2];
char *argv2[MAX/2];
char line[MAX];
int numberOfArguments;
int status;
pid_t pid, died;
char *path;


//Reads from the user line
void readLine(void) {
  int i = 0;
  while ((i < MAX) && ((line[i] = getchar()) != '\n'))
      i++;
  //if reading extends MAX, the additional arguments are cut
  if (i == MAX) {
      line[MAX-1] = '\0';
      while (getchar() != '\n');
  }
  else
      line[i] = '\0';
  argv[0] = strtok(line, " ");
  numberOfArguments = 1;
  argv[numberOfArguments] = strtok(NULL, " ");
  while (argv[numberOfArguments] != NULL) {
      numberOfArguments++;
      argv[numberOfArguments] = strtok(NULL, " ");
  }
}

//Splits one command with arguments into two commands with arguments
void extractCommand(char* symbol) {
  int i = 0;
  int count = 0;
  for (i = 0; argv[i] != NULL; i++)
      if (!strcmp(argv[i], symbol)) {
         argv[i] = NULL;
         while (argv[i+1] != NULL) {
               argv2[count] = argv[i+1];
               argv[i+1] = NULL;
               i++;
               count++;
         }
      }
} 

//Redirects the standard output into a filename
void outRedirection(void) {
  extractCommand(">");
  int fd;
  if ((pid = fork()) == -1) {
      perror("fork"); 
      exit(1);
  }
  if (pid == 0) {
      close(1);
      fd = creat(argv2[0], 0644);
      execvp(argv[0], argv); 
      perror("execv");
      exit(1);
  }
  if (pid != 0) {
      wait(NULL);
      printf("Done ");
      printf(argv[0]);
      printf(".\n");
  }
}

//Reads from a file as standard input
void inRedirection(void) {
  extractCommand("<");
  int fd;
  if ((pid = fork()) == -1) {
      perror("fork"); 
      exit(1);
  }
  if (pid == 0) {
      close(0);
      //open the file argv2[0] and use it as standard input
      fd = open(argv2[0], O_RDONLY); 
      execvp(argv[0], argv); 
      perror("execv");
      exit(1);
  }
  if (pid != 0) {
      wait(NULL);
      printf("Done ");
      printf(argv[0]);
      printf(".\n");
  }
}

//Function where a child forks again and creates another child
void backgrounding(void) {
  extractCommand("&");
  // the child forks again
  switch (pid = fork()) {
    case -1:
      // something went wrong
      printf("panic: can't fork\n");
      exit(-1);
    case 0:
      // this is the code the grandchild runs
      execvp(argv[0], argv); 
      printf("Command %s not found\n", argv[0]);
      exit(1);
    default:
      // this  is the code the child runs => it just exits, so
      // the call to wait() in the parent will return;
      // the grandchild will be adopted by the init process.
      usleep(10000); // sleep 10000 micro seconds
      exit(0);
  }
}

//Connects the standard output of one command to standard input of another command
void piping(void) {
  extractCommand("|");
  int fd[2];
  if (pipe(fd) < 0)
     printf("Cannot get a pipe\n");
  if ((pid = fork()) == 0) {
     close(1);                 
     dup(fd[1]);
     close(fd[0]);
     execvp(argv[0], argv); 
  } else if (pid > 0) {
            close(0);
            dup(fd[0]);
            close(fd[1]);
            execvp(argv2[0], argv2);
         } else 
            printf("Unable to fork\n");                                         
}

//Executes a simple command or calls the piping or backgrounding function
void process(int type) {
  switch (pid = fork()) {
    case -1:
      printf("panic: can't fork\n");
      exit(-1);
    case 0: 
  	  if (type == 0) {
          execvp(argv[0], argv);
	      printf("Command %s not found\n", argv[0]);
	      exit(0);
	  } else if (type == 1)
	      backgrounding();
	  else if (type == 2)
          piping();
    default:
      died = wait(&status);
    }
}

//Sets a new environment variable or overwrites an existing environment variables
void setEnv(int n) {
  int returnValue;
  char* name;
  char* value;
  name = strtok(argv[n], "=");
  value = strtok(NULL, " ");
  returnValue = setenv(name, value, 1);
  if (returnValue == 0)
      printf("Environment set successfully\n");
  else
      printf("Error in setting environment\n");
}

//Deletes an environment variable from the environment
void unsetEnv(void) {
  int returnValue;
  returnValue = unsetenv(argv[1]);
  if (returnValue == 0)
      printf("Environment variable %s unset successfully\n", argv[1]);
  else
      printf("Error in unsetting environment\n");
}

//Changes the current directory
void changeDirectory(void) {
  int returnValue;
  char* path;
  setenv("OLDPWD", getenv("PWD"), 1);
  if (argv[1] != NULL) {
      returnValue = chdir(argv[1]);
      setenv("PWD", (char*)get_current_dir_name(),1);
  } else {
      path = getenv("HOME");
      returnValue = chdir(path);
      setenv("PWD", getenv("HOME"), 1);
  }
  if (returnValue != 0)
      printf("Error in changing directory\n");
  else printf("Current directory: %s\n", getenv("PWD"));
}

//Checks whether the array of pointers to strings contains a special symbol
char stringCompare(void) {
  int i;
  char c;
  for (i = 0; argv[i] != '\0'; i++) {
      if (!strcmp(argv[i], ">"))
  	 return argv[i][0];
      if (!strcmp(argv[i], "<"))
         return argv[i][0];
      if (!strcmp(argv[i], "|"))
         return argv[i][0];
      if (!strcmp(argv[i], "&"))
         return argv[i][0];
  }
  return '\0';
}

int main(void) { 
  printf("Initializing shell: \n");
  while (TRUE) {
      printf("myshell$ ");
      readLine();
       
      //Continue if only the "Enter" key was pressed
      if (argv[0] == NULL)
         continue;
         
      //Exit from shell if command is equal "exit"
      if (!strcmp(argv[0], "exit")) 
      	exit(0);
 	  
      	
      //Check for environment related commands (i.e. export USER=test, USER=test, unset USER)
      if (!strcmp(argv[0], "export"))
        setEnv(1);
      else if (strchr(argv[0], '='))
        setEnv(0);
      else if (!strcmp(argv[0], "unset"))
        unsetEnv();
        
      //Calls the function for changing the current directory
      else if (!strcmp(argv[0], "cd"))
        changeDirectory();
        
      //Calls the function for redirection, piping, backgrounding or for executing commands 
      else switch (stringCompare()) {
              case '<': inRedirection();
                        break;
  
              case '>': outRedirection();
                        break;
               
              case '|': process(2);
                        break;
                        
              case '&': process(1);
                        break;
              
              default: process(0);
                       break;
       }
    }
    return 0;
}
