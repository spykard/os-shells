#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "display.h"

#include <pthread.h>

pthread_mutex_t mut;

struct mythreadsdata // Struct for the parameters that we pass via thread
{	char *text;};

void *functi(void *keimeno) // The function that the threads will execute
{
	pthread_mutex_lock(&mut);

	struct mythreadsdata *data = keimeno;
	int i;
	for (i=0;i<10;i++)
		display(data->text);

	free(data);
	pthread_mutex_unlock(&mut);

	return NULL;
}

int main()
{
	int x,y;
	struct mythreadsdata *data;
	char *text1, *text2;
	text1 = "Hello world\n";
	text2 = "Kalimera kosme\n";

	pthread_t id1, id2;
	pthread_attr_t attr;
	pthread_attr_init(&attr);	
	pthread_mutexattr_t attr2;
	pthread_mutexattr_init(&attr2);

	if (pthread_mutex_init(&mut, &attr2) != 0) // Mutex for sync
	{
		printf("Error mutex_init\n");
		exit(10);}

  	data = malloc(sizeof(struct mythreadsdata));
 	data->text = text1;
	x = pthread_create(&id1, &attr, functi, data);

	data = malloc(sizeof(struct mythreadsdata));
	data->text = text2;	
	y = pthread_create(&id2, &attr, functi, data);

	if (x) // If not 0 then it's error from pthread_create
	{
		printf("Error pthread_create - Code %d\n", x);
		exit(98);}
	if (y) // If not 0 then it's error from pthread_create
	{
		printf("Error pthread_create - Code %d\n", y);
		exit(99);}

	pthread_join(id1, NULL);
	pthread_join(id2, NULL);
	pthread_mutex_destroy(&mut);

	return 0;
}
