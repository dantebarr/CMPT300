#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h> 

int treasure = 30;
int winProb; //probabilty a visitor will NOT be defeated, i.e. will win

int maximumHunterInterval;
int maximumThiefInterval;
int huntersOnPath = 0;
int thievesOnPath = 0;

bool smaugSatisfied = false;

pthread_mutex_t hOP;
pthread_mutex_t tOP;

sem_t pathHunter;
sem_t pathThief;

void smaug();
void hunter();
void thief();

int main(){
    int seed, nextHunterTime, nextThiefTime;
    pid_t smaugpid, hunterpid, thiefpid;
    clock_t startTime, elapsedTime;
	
	//initialize threads
	int pshared = 1; //this tells the semaphore to be shared accross processes not just threads
	int seminitval = 0;
	sem_init(&pathHunter, pshared, seminitval);
	sem_init(&pathThief, pshared, seminitval);
	
	//initalize mutexes
	pthread_mutex_init(&hOP, NULL);
	pthread_mutex_init(&tOP, NULL);
	
    printf("Enter a value for the maximum Hunter Interval in seconds : ");
    scanf("%d", &maximumHunterInterval);

    printf("Enter a value for the maximum Thief Interval in seconds : ");
    scanf("%d", &maximumThiefInterval);

    printf("Enter the win probability percentage : ");
    scanf("%d", &winProb);

    printf("Enter the seed with which to generate random Thief and Hunter Intervals : ");
    scanf("%d", &seed);

    srand(seed);

    //spawn Smaug
    if((smaugpid = fork()) < 0){
        printf("Smaug process failed to spawn, terminating program\n");
        abort();
    }
	else if(smaugpid == 0){
		//All work done by Smaug here
		smaug();
		exit(0);
	}

    nextHunterTime = rand()%maximumHunterInterval;
    nextThiefTime = rand()%maximumThiefInterval;

	//sets inital start time
    startTime = clock();

    while(!smaugSatisfied){ //while one of the exit conditions isnt met
		//calculates time since startTime initialized in seconds (microseconds*1000,000)
        elapsedTime = (clock() - startTime)/CLOCKS_PER_SEC * 1000000;
		if(elapsedTime >= nextHunterTime){
			//generate new hunter here
			if((hunterpid = fork()) < 0){
				printf("Hunter process failed to spawn, terminating program\n");
				abort();
			}
			else if(hunterpid == 0){
				hunter();
				exit(0);
			}
			//calculate how far in the future the next thief will spawn
			nextHunterTime = elapsedTime + rand()%maximumHunterInterval;
		}
		if(elapsedTime >= nextThiefTime){
			//generate new thief here
			if((thiefpid = fork()) < 0){
				printf("Thief process failed to spawn, terminating program\n");
				abort();
			}
			else if(thiefpid == 0){
				thief();
				exit(0);
			}
			//calculate how far in the future the next thief will spawn
			nextThiefTime = elapsedTime + rand()%maximumThiefInterval;
		}
    }
	
	pthread_mutex_destroy(&hOP);
	pthread_mutex_destroy(&tOP);
	
	sem_destroy(&pathHunter);
    sem_destroy(&pathThief);
	
    return 0;
}


void smaug(){
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	return;
}


void hunter(){
	pthread_mutex_lock(&hOP);
	huntersOnPath++;
	pthread_mutex_unlock(&hOP);
	
	//here the thread now waits for Smaug to release it
	sem_wait(&pathHunter);
	
	
	return;
}

void theif(){
	pthread_mutex_lock(&tOP);
	theivesOnPath++;
	pthread_mutex_unlock(&tOP);
	
	//here the thread now waits for Smaug to release it
	sem_wait(&pathThief);
	
	
	return;
}























