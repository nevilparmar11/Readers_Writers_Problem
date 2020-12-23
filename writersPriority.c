/*
* @Author: nevil11
* @Date:   2020-09-17 09:55:41
* @Last Modified by:   nevil11
* @Last Modified time: 2020-09-17 10:35:59
*/
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<limits.h>

int readcount = 0, writecount = 0;
sem_t rsem, wsem ;
pthread_mutex_t x, y, z ;
int cnt = 1;


/* Function Signatures */
void doReading(void *rno);
void doWriting(void *wno);
void reader(void *rno);
void writer(void *wno);

// In real application we can perform some DB operations
void doReading(void *rno) {
	// Reading Section
	printf("Reader %d: read cnt as %d\n", *((int *)rno), cnt);
}

void doWriting(void *wno) {
	// since we are running it inside the infinite loop , the value of cnt may go beyond the limit of 32 bit int
	if ( ((long long int)cnt * 2) > INT_MAX)
		cnt = 1;
	else
		cnt = cnt * 2;
	printf("Writer %d: modified cnt to %d\n", (*((int *)wno)), cnt);
}

void reader(void *rno) {
	while (1) {

		pthread_mutex_lock(&z);
		sem_wait(&rsem);
		pthread_mutex_lock(&x);
		readcount++;
		if (readcount == 1)
			sem_wait(&wsem);
		pthread_mutex_unlock(&x);
		sem_post(&rsem);
		pthread_mutex_unlock(&z);

		doReading(rno);
		
		pthread_mutex_lock(&x);
		readcount--;
		if (readcount == 0)
			sem_post(&wsem);
		pthread_mutex_unlock(&x);

		// sleep is used just to slow down the output on the terminal
		sleep(2);

	}
}

void writer(void *wno) {
	while (1) {

		pthread_mutex_lock(&y);
		writecount++;
		if (writecount == 1)
			sem_wait(&rsem);
		pthread_mutex_unlock(&y);

		sem_wait(&wsem);
		doWriting(wno);
		sem_post(&wsem);

		pthread_mutex_lock(&y);
		writecount--;
		if (writecount == 0)
			sem_post(&rsem);
		pthread_mutex_unlock(&y);

		// sleep is used just to slow down the output on the terminal
		sleep(2);

	}
}

int main()
{

	pthread_t read[10], write[5];
	sem_init(&wsem, 0, 1);
	pthread_mutex_init(&z, NULL);
	pthread_mutex_init(&y, NULL);
	pthread_mutex_init(&x, NULL);
	sem_init(&rsem, 0, 1);

	int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}; //Just used for numbering the producer and consumer

	for (int i = 0; i < 10; i++) {
		pthread_create(&read[i], NULL, (void *)reader, (void *)&a[i]);
	}
	for (int i = 0; i < 5; i++) {
		pthread_create(&write[i], NULL, (void *)writer, (void *)&a[i]);
	}

	for (int i = 0; i < 10; i++) {
		pthread_join(read[i], NULL);
	}
	for (int i = 0; i < 5; i++) {
		pthread_join(write[i], NULL);
	}

	sem_destroy(&wsem);
	pthread_mutex_destroy(&z);
	sem_destroy(&rsem);
	pthread_mutex_destroy(&y);
	pthread_mutex_destroy(&x);

	return EXIT_SUCCESS;

}