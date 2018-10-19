// Aaron T Johnson - atj0015@unt.edu
// CSCE 4600 Project 1

#include <pthread.h>	// thread creation
#include <stdio.h>		// I/O
#include <semaphore.h>	// semaphore creation
//#include <string.h>	// is this library necessary here? no string reading?
#include <errno.h>		// error handling
#include <stdlib.h>		// needed for rand()

/* the maximum time (in seconds) to sleep */
#define MAX_SLEEP_TIME	5

/* number of maximum waiting students */
#define MAX_WAITING_STUDENTS	3

/* number of potential students */
#define NUM_OF_STUDENTS		5

/* number of available seats */
#define NUM_OF_SEATS	3

/* number of helps needed */
#define NUM_OF_HELPS	3

/* mutex lock */
pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

/* semaphore declarations */
sem_t students_sem;	// a student in ta's office
sem_t ta_sem;		// just help a student
sem_t chairSem[3];
sem_t chair_in_office_sem;

/* the number of waiting students */
int waiting_students;

/* student being served */
int student_number;
int studentsHelped = 0;
int studentHelped[5];

/* the numeric id of each student */
int student_id[NUM_OF_STUDENTS];

// Open chairs, and chair index
int chairCount = 0;
int CurrentIndex = 0;

pthread_t ta;
pthread_t students[NUM_OF_STUDENTS];

void *taHelping();
void *studentProgramming(void *studentID);
void randSleep(void);

int main(void) {
	int i;
	for (i=0; i<5; i++)
		studentHelped[i]=0;
	/**
 	* Initialize all relevant data structures and
 	* synchronization objects.
 	*/
	sem_init(&ta_sem, 0, 0);
	sem_init(&students_sem, 0, 0);
	for (i=0; i<3; i++)
		sem_init(&chairSem[i], 0, 0);
		
	pthread_mutex_init(&mutex_lock, NULL);
	
	pthread_create(&ta, NULL, taHelping, NULL);
	for (i=0; i<5; i++)
		pthread_create(&students[i], NULL, studentProgramming, (void*) (long)i);
	
	pthread_join(ta, NULL);
	for(i=0; i<5; i++)
		pthread_join(students[i], NULL);

	/* when all students have finished, we will cancel the TA thread */	
	if (pthread_cancel(ta) != 0)
		printf("%d\n",strerror(errno));
	return 0;
}

void *taHelping(){
	while(1){
		// TA is sleeping, waiting for students
		sem_wait(&ta_sem);
		printf("TA is asleep...\n");
		
		while (1){
			// mutex lock
			pthread_mutex_lock(&mutex_lock);
			if (chairCount==0){
				// if chairs are empty, break TA loop
				pthread_mutex_unlock(&mutex_lock);
				break;
			}
			// TA brings in next student in chair
			sem_post(&chairSem[CurrentIndex]);
			chairCount--;
			printf("A student has vacated a chair. Chairs available: %d\n", 3-chairCount);
			CurrentIndex = (CurrentIndex+1)%3;
			pthread_mutex_unlock(&mutex_lock);
			
			printf("\t TA is currently helping a student.\n");
			studentsHelped++;
			sleep(5);
			sem_post(&students_sem);
			sleep(10);
			if(studentsHelped==15){
				printf("TA is going home for the day...");
				return 0;
			}
		}
		break;
	}
}

void *studentProgramming(void *studentID){

	while(1){
		printf("Student %ld is working on programming assignment.\n", (long)studentID);
		randSleep();
		
		printf("Student %ld needs help from the TA.\n", (long)studentID);
		
		pthread_mutex_lock(&mutex_lock);
		int count = chairCount;
		pthread_mutex_unlock(&mutex_lock);
		
		if(count<3){
			if (count==0)
					sem_post(&ta_sem);
			else
				printf("Student %ld sat on a chair waiting for the TA.\n", (long)studentID);
			
			pthread_mutex_lock(&mutex_lock);
			int chairIndex = (CurrentIndex + chairCount)%3;
			chairCount++;
			printf("Student sat in chair. Chairs remaining: %d\n", 3-chairCount);
			pthread_mutex_unlock(&mutex_lock);
			
			sem_wait(&chairSem[CurrentIndex]);
			printf("\t Student %ld is getting help from the TA.\n", (long)studentID);
			studentHelped[(int)studentID]++;
			printf("\tStudent %ld has been helped %d times.\n", (long)studentID, studentHelped[(int)studentID]);
			sem_wait(&students_sem);
			printf("Student %ld left the TA's office.\n", (long)studentID);
			if (studentHelped[(int)studentID]==3){
				printf("Student %ld is done working, and is going home for the day...\n", (long)studentID);
				break;
			}
		}
			else // if no chairs are available
				printf("Student %ld will come back later.\n", (long)studentID);
	}
}

void randSleep(void){
	int time = rand() % MAX_SLEEP_TIME +1;
	sleep(time);
}