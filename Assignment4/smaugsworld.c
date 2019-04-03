#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h> 
#include <sys/types.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

int winProb; //probabilty a visitor will NOT be defeated, i.e. will win

pthread_mutex_t hOP;
pthread_mutex_t tOP;
pthread_mutex_t sS;

sem_t* pathHunter;
sem_t* pathThief;
sem_t* smaugSleep;
sem_t* smaugBusy;

void smaug(int* treasure, int* smaugSatisfied, int* huntersOnPath, int* thievesOnPath);
void hunter(int* treasure, int* smaugSatisfied, int* huntersOnPath, int* huntersDefeated);
void thief(int* treasure, int* smaugSatisfied, int* thievesOnPath, int* thievesDefeated);
void* csm(size_t size);

int main(){
    int maximumHunterInterval, maximumThiefInterval, seed, nextHunterTime, nextThiefTime, i;
    pid_t smaugpid, hunterpid, thiefpid;
    clock_t startTime, elapsedTime;
	//initialising global mem to be shared between processes
	int tr = 30;
	int hunP = 0;
	int thiP = 0;
	int smS = 0;
	int tD = 0;
	int hD = 0;

	int* huntersDefeated = csm(sizeof(int));
	memcpy(huntersDefeated, &hD, sizeof(int));

	int* thievesDefeated = csm(sizeof(int));
	memcpy(thievesDefeated, &tD, sizeof(int));

	int* treasure = csm(sizeof(int));
	memcpy(treasure, &tr, sizeof(int));

	int* huntersOnPath = csm(sizeof(int));
	memcpy(huntersOnPath, &hunP, sizeof(int));
	
	int* thievesOnPath = csm(sizeof(int));
	memcpy(thievesOnPath, &thiP, sizeof(int));
	
	int* smaugSatisfied = csm(sizeof(int));
	memcpy(smaugSatisfied, &smS, sizeof(int));

	pathHunter = sem_open("pH", O_CREAT, 0600, 0); 
	pathThief = sem_open("pT", O_CREAT, 0600, 0);
	smaugSleep = sem_open("sS", O_CREAT, 0600, 0);
	smaugBusy = sem_open("sB", O_CREAT, 0600, 0);
	
	
	//initialize semaphores
	int pshared = 1; //this tells the semaphore to be shared accross processes not just threads
	int seminitval = 0;
	sem_init(pathHunter, pshared, seminitval);
	sem_init(pathThief, pshared, seminitval);
	sem_init(smaugSleep, pshared, seminitval);
	sem_init(smaugBusy, pshared, seminitval);
	
	
	//initalize mutexes
	pthread_mutex_init(&hOP, NULL);
	pthread_mutex_init(&tOP, NULL);
	pthread_mutex_init(&sS, NULL);
	
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
		smaug(treasure, smaugSatisfied, huntersOnPath, thievesOnPath);
		exit(0);
	}

    nextHunterTime = rand()%maximumHunterInterval;
    nextThiefTime = rand()%maximumThiefInterval;

	//sets inital start time
    startTime = clock();
	
	
	/*terminate program conditions
	Smaug has defeated 4 treasure hunters or 3 thieves
	Smaug has no treasure
	Smaug has 80 jewels
	*/
    while(!(*smaugSatisfied)){ //while one of the exit conditions isnt met
		//calculates time since startTime initialized in seconds (microseconds*1000,000)
        elapsedTime = (clock() - startTime) /CLOCKS_PER_SEC;
		if(elapsedTime >= nextHunterTime){
			//generate new hunter here
			if((hunterpid = fork()) < 0){
				printf("Treasure Hunter process failed to spawn, terminating program\n");
				abort();
			}
			else if(hunterpid == 0){
				hunter(treasure, smaugSatisfied, huntersOnPath, huntersDefeated);
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
				thief(treasure, smaugSatisfied, thievesOnPath, thievesDefeated);
				exit(0);
			}
			//calculate how far in the future the next thief will spawn
			nextThiefTime = elapsedTime + rand()%maximumThiefInterval;
		}
    }
	
	for(i = 0; i < *huntersOnPath; i++){
		sem_post(pathHunter);
	}
	for(i = 0; i < *thievesOnPath; i++){
		sem_post(pathThief);
	}
	
	pthread_mutex_destroy(&hOP);
	pthread_mutex_destroy(&tOP);
	pthread_mutex_destroy(&sS);
	
	sem_destroy(pathHunter);
    sem_destroy(pathThief);
	sem_destroy(smaugSleep);
	sem_destroy(smaugBusy);
	
    return 0;
}

void* csm(size_t size){
	int protection = PROT_READ | PROT_WRITE;
	int visibility = MAP_ANONYMOUS | MAP_SHARED;

	return mmap(NULL, size, protection, visibility, 0, 0);
	
}

void smaug(int* treasure, int* smaugSatisfied, int* huntersOnPath, int* thievesOnPath){
	int sval, huntersDefeated = 0, thievesDefeated = 0;
	
	while(!(*smaugSatisfied)){
		
		printf("Smaug is going to sleep\n");
		
		sem_wait(smaugSleep);
	
		printf("Smaug has been woken up\n");
	
		if(*thievesOnPath > 0){//if there is a thief on the path it takes priority
			printf("Smaug smells a thief\n");
			sem_post(pathThief);
			sem_wait(smaugBusy);
			printf("Smaug has finished a game\n");
		}
		else{//if there is a hunter
			printf("Smaug smells a treasure hunter\n");
			sem_post(pathHunter);
			sem_wait(smaugBusy);
			printf("Smaug has finished a battle\n");
		}
		
		if(*treasure <= 0 && !(*smaugSatisfied)){
			*smaugSatisfied = 1;
			printf("Smaug has no more treasure\n");
		}
		else if(*treasure >= 80 && !(*smaugSatisfied)){
			*smaugSatisfied = 1;
			printf("Smaug has amassed 80 jewels in treasure\n");
		}
	}
		
	return;
}


void hunter(int* treasure, int* smaugSatisfied, int* huntersOnPath, int* huntersDefeated){
	pid_t myPID = getpid();
	
	if(*smaugSatisfied){
		exit(0);
	}

	printf("Treasure hunter %d wandering the valley\n", myPID);
	printf("Treasure hunter %d is travelling to the valley\n", myPID);
	
	pthread_mutex_lock(&hOP);
	*huntersOnPath += 1;
	pthread_mutex_unlock(&hOP);
	
	//tell Smaug he can wake up as there is a visitor
	//pthread_mutex_lock(&sS);
	sem_post(smaugSleep);
	//pthread_mutex_unlock(&sS);
	
	//here the thread now waits for Smaug to release it
	sem_wait(pathHunter);
	
	
	if(*smaugSatisfied){
		exit(0);
	}
	
	pthread_mutex_lock(&hOP);
	*huntersOnPath -= 1;
	pthread_mutex_unlock(&hOP);
	
	//fight smaug
	printf("Treasure hunter %d is fighting Smaug\n", myPID);
	
	if((rand()%100+1) < winProb){//if wins
		printf("Treasure hunter %d has won and recieves treasure\n", myPID);
		*treasure -= 10;
		if(*treasure < 0){
			*treasure = 0;
		}
		printf("Smaug has been defeated by a treasure hunter\n");
		printf("Smaug has lost some treasure, he now has %d jewels\n", *treasure);
	}
	else{//else loses
		*huntersDefeated += 1;
		printf("\n hd = %d \n\n", *huntersDefeated);
		printf("Treasure hunter %d has been defeated and pays the price\n", myPID);
		*treasure += 5;
		printf("Smaug has defeated a treasure hunter\n");
		printf("Smaug has added to his treasure, he now has %d jewels\n", *treasure);
	}
	
	if(*huntersDefeated >= 4){
		*smaugSatisfied = 1;
		printf("Smaug has defeated 4 treasure hunters\n");
	}
	//tell Smaug he can go back to sleep
	sem_post(smaugBusy);
	
	return;
}

void thief(int* treasure, int* smaugSatisfied, int* thievesOnPath, int* thievesDefeated){
	pid_t myPID = getpid();
	
	if(*smaugSatisfied){
		exit(0);
	}

	printf("Thief %d wandering the valley\n", myPID);
	printf("Thief %d is travelling to the valley\n", myPID);
	
	pthread_mutex_lock(&tOP);
	*thievesOnPath += 1;
	pthread_mutex_unlock(&tOP);
	
	//tell Smaug he can wake up as there is a visitor
	//pthread_mutex_lock(&sS);
	sem_post(smaugSleep);
	//pthread_mutex_unlock(&sS);
	
	//here the thread now waits for Smaug to release it
	sem_wait(pathThief);
	
	if(*smaugSatisfied){
		exit(0);
	}
	
	pthread_mutex_lock(&tOP);
	*thievesOnPath -= 1;
	pthread_mutex_unlock(&tOP);
	
	//play with smaug
	printf("Thief %d is playing with Smaug\n", myPID);
	
	if((rand()%100+1) < winProb){//if wins
		printf("Thief %d has won and recieves treasure\n", myPID);
		*treasure -= 8;
		if(*treasure < 0){
			*treasure = 0;
		}
		printf("Smaug has been defeated by a thief\n");
		printf("Smaug has lost some treasure, he now has %d jewels\n", *treasure);
	}
	else{//else loses
		*thievesDefeated += 1;
		printf("\n td = %d \n\n",*thievesDefeated);
		printf("Thief %d has been defeated and pays the price\n", myPID);
		*treasure += 20;
		printf("Smaug has defeated a thief\n");
		printf("Smaug has added to his treasure, he now has %d jewels\n", *treasure);
	}
	
	if(*thievesDefeated >= 3){
		*smaugSatisfied = 1;
		printf("Smaug has defeated 3 thieves\n");
	}
	
	//tell Smaug he can go back to sleep
	sem_post(smaugBusy);
	
	return;
}























