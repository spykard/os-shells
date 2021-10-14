#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "display.h"

#include <pthread.h>

enum { STATE_A, STATE_B } state = STATE_A; // STATE_A Thread A next, STATE_B Thread B next
pthread_cond_t condA;
pthread_cond_t condB;
pthread_mutex_t mut;

struct mythreadsdata // Struct for the parameters that we pass via thread
{	char *text;};

// In order to wake up from thread to thread we need distinct functions
void *functA(void *keimeno) // The function that thread A will execute
{
	struct mythreadsdata *data = keimeno;
	int i;
	for (i=0;i<10;i++)
	{
		pthread_mutex_lock(&mut);
		while (state != STATE_A) // Wait for A
			pthread_cond_wait(&condA, &mut);
		pthread_mutex_unlock(&mut);		

		display(data->text);

		pthread_mutex_lock(&mut); // Wake up B
		state = STATE_B;
		pthread_cond_signal(&condB);
		pthread_mutex_unlock(&mut);
	}
	free(data);
	return NULL;
}

void *functB(void *keimeno) // The function that thread A will execute
{
	struct mythreadsdata *data = keimeno;
	int i;
	for (i=0;i<10;i++)
	{
		pthread_mutex_lock(&mut);
		while (state != STATE_B) // Wait for B
			pthread_cond_wait(&condB, &mut);
		pthread_mutex_unlock(&mut);

		display(data->text);

		pthread_mutex_lock(&mut); // Wake up A
		state = STATE_A;
		pthread_cond_signal(&condA);
		pthread_mutex_unlock(&mut);
	}
	free(data);
	return NULL;
}

int main()
{
	int x,y;
	struct mythreadsdata *data;
	char *text1, *text2;
	text1 = "ab";
	text2 = "cd\n";

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
	x = pthread_create(&id1, &attr, functA, data);

	data = malloc(sizeof(struct mythreadsdata));
	data->text = text2;	
	y = pthread_create(&id2, &attr, functB, data);

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
