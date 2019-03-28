#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>   // clock_gettime, CLOCK_REALTIME



void *CreateVehicle( void *ptr );
void *Truck( void *ptr );
void *Car( void *ptr );
void *Captain(void *ptr);

int sailing = 1;//equals true

int V = 0; // Percent chance vehicle will be a truck
int K = 0; // the minimum time to wait before creating the next vehicle thread
int SEED = 0; // random # seed

int carsWaiting = 0; 
int carsLoaded = 0;
int carsSailing = 0;
int carsUnloaded = 0;

int trucksWaiting = 0; 
int trucksLoaded = 0;
int truckSailing = 0;
int truckUnloaded = 0;

pthread_mutex_t cWm;
pthread_mutex_t cLm;
pthread_mutex_t cSm;
pthread_mutex_t cUm;

pthread_mutex_t tWm;
pthread_mutex_t tLm;
pthread_mutex_t tSm;
pthread_mutex_t tUm;

sem_t cWs;
sem_t cLs;
sem_t cSs;
sem_t cUs;

sem_t tWs;
sem_t tLs;
sem_t tSs;
sem_t tUs;

sem_t cEs;
sem_t tEs;

sem_t capCw;
sem_t capTw;

int main()
{
    pthread_t createVehicle, captain; 
    int thread1, thread2; 
	//sem_init returns 0 if successful, might be worth putting in a catch here
	int pshared = 1, seminit = 0;
	sem_init(&cWs, pshared, seminit);
	sem_init(&cLs, pshared, seminit);
	sem_init(&cSs, pshared, seminit);
	sem_init(&cUs, pshared, seminit);
	sem_init(&cWs, pshared, seminit);
	sem_init(&cLs, pshared, seminit);
   	sem_init(&cSs, pshared, seminit);
 	sem_init(&cUs, pshared, seminit);
    sem_init(&cEs, pshared, seminit);
    sem_init(&tEs, pshared, seminit);

    sem_init(&capCw, pshared, seminit);
    sem_init(&capTw, pshared, seminit);


    printf("Please enter integer values for the following variables\n");

    printf("Enter the percent probability that the next vehicle is a truck\n");
    fflush( stdout ); // flush to ensure buffer is clear
    scanf("%d", &V);

    printf("Enter the maximum length of the interval between vehicles time interval should be >1000\n");
    fflush( stdout ); // flush to ensure buffer is clear
    scanf("%d", &K);

    printf("Enter the seed for random number generation\n");
    fflush( stdout ); // flush to ensure buffer is clear
    scanf("%d", &SEED);


    /*  creating & joining createVehicle thread */
    thread1 = pthread_create( &createVehicle, NULL, CreateVehicle, NULL);
    if(thread1)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",thread1);
	fflush( stdout );
        exit(EXIT_FAILURE);
    }
    
   

    /*  creating & joining captain thread */
    thread2 = pthread_create( &captain, NULL, Captain, NULL);
    if(thread2)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",thread2);
        exit(EXIT_FAILURE);
    }
    pthread_join( createVehicle, NULL);//moved this line from above as I believe it would stop the main thread from creating the captain -d
    pthread_join( captain, NULL);
    
    //destroy mutexes
    pthread_mutex_destroy(&cWm);
    pthread_mutex_destroy(&cLm);
    pthread_mutex_destroy(&cSm);
    pthread_mutex_destroy(&cUm);
    pthread_mutex_destroy(&tWm);
    pthread_mutex_destroy(&tLm);
    pthread_mutex_destroy(&tSm);
    pthread_mutex_destroy(&tUm);

    //destroy semaphores
    sem_destroy(&cWs);
    sem_destroy(&cLs);
    sem_destroy(&cSs);
    sem_destroy(&cUs);
    sem_destroy(&tWs);
    sem_destroy(&tLs);
    sem_destroy(&tSs);
    sem_destroy(&tUs);

    return 0;
    // ...
}


/* 
    Thread responsable for creating all of the cars and trucks
*/
void *CreateVehicle( void *ptr )
{
    pthread_t thread_id[10000]; // threads/vehicles
    srand(SEED); // initialize rng using global SEED
    clock_t startTime, elapsedTime, waitTime;//next arrival will just be elapsedTime + wait time
    int v_prob, i = 0;

    printf("CREATEVEHICLE: Vehicle creation thread has been started\n");
    fflush( stdout ); // flush to ensure buffer is clear

    startTime = clock(); // starting timer
    waitTime = startTime*1000/CLOCKS_PER_SEC;//initial first arrival time is at 0


    /* Creating vehicle threads */
    //make this run until captain signals 5 loads have unloaded or until 10000 vehicles
    //have been created, this should never happen though as only 5*6 need to sail
    while(sailing && i < 10000)
    {   

        
            //I changed this from 100 + 1 since it says 0-100 which is 101 numbers including 0
        do{
            //maybe we should find a different way for this thread to idle instead of continually recalculating the elapsed time-d
            elapsedTime = (clock() - startTime)*1000/CLOCKS_PER_SEC; // getting elapsed time compared to startTime in msec
        }while(elapsedTime < waitTime);
        
        
        if(sailing){
		printf("CREATEVEHICLE: Elapsed time %ld msec\n", waitTime);
		fflush( stdout );
	}
        

        v_prob = rand() % 101; // probability that vehicle is a car or truck

        if ( v_prob <= V && sailing )  // if vehicle is a truck
        {
            pthread_create( &thread_id[i], NULL, Truck, NULL );
            i++;
            printf("CREATEVEHICLE: Created a truck thread\n");
		fflush( stdout );
	
        }
        else // if vehicle is a car
        {
	if(sailing){
            pthread_create( &thread_id[i], NULL, Car, NULL );
            i++;
	
            printf("CREATEVEHICLE: Created a car thread\n");
		fflush( stdout );
	}
        }

        waitTime = clock()*1000/CLOCKS_PER_SEC + ((rand() % (K - 1000)) + 1001);  // take current time and add random wait time to it
	if(sailing){
        	printf("CREATEVEHICLE: Next arrival time %ld msec\n", waitTime);
		fflush( stdout );
	}
        //usleep(waitTime*1000); I made the while loop above instead of the usleep although this may be better im fine either way -d

    }
	for(int j = 0; j < i; j++)
    {
        pthread_exit( &thread_id[j]); 
    }

    /* Joining vehicle threads **Required** */
    for(int j = 0; j < i; j++)
    {
        pthread_join( thread_id[j], NULL); 
    }

    
}



void *Truck( void *ptr )
{
    pthread_t self;

    self = pthread_self();
    printf("TRUCK: Truck with threadID %ld queued\n", self);
	fflush( stdout );

    pthread_mutex_lock(&tWm);
    trucksWaiting++;
    pthread_mutex_unlock(&tWm); //unlock critical section

    sem_wait(&tWs); //place truck in blocked queue of semaphore to await signal from el capitan

    printf("TRUCK: Truck with threadID %ld leaving the queue to load\n", self);
	fflush( stdout );

    pthread_mutex_lock(&tLm);
    trucksLoaded++;
    
    pthread_mutex_unlock(&tLm); //unlock critical section

    sem_wait(&tLs); //place truck in blocked queue of semaphore to await signal from el capitan

    pthread_mutex_lock(&tLm);
    trucksLoaded--;
    pthread_mutex_unlock(&tLm); //unlock critical section

    printf("TRUCK: Truck with threadID %ld is onboard the ferry\n", self);
	fflush( stdout );

    pthread_mutex_lock(&tSm);
    truckSailing++;
    pthread_mutex_unlock(&tSm); //unlock critical section

    sem_wait(&tSs);

    pthread_mutex_lock(&tSm);
    truckSailing--;
    pthread_mutex_unlock(&tSm); //unlock critical section

    printf("TRUCK: Truck with threadID %ld is now unloading\n", self);
	fflush( stdout );

    pthread_mutex_lock(&tUm);
    truckUnloaded++;
    pthread_mutex_unlock(&tUm); //unlock critical section

    sem_wait(&tUs);

    pthread_mutex_lock(&tUm);
    truckUnloaded--;
    pthread_mutex_unlock(&tUm); //unlock critical section

    printf("TRUCK: Truck with threadID %ld has unloaded\n", self);
	fflush( stdout );

	sem_wait(&tEs);

    printf("TRUCK: Truck with threadID %ld is about to exit\n", self);
	fflush( stdout );

}




void *Car( void *ptr )
{
    pthread_t self;

    self = pthread_self();
    printf("CAR: Car with threadID %ld queued\n", self);
	fflush( stdout );

    pthread_mutex_lock(&cWm);
    carsWaiting++;
    pthread_mutex_unlock(&cWm); //unlock critical section

    sem_wait(&cWs); //place car in blocked queue of semaphore to await signal from el capitan

    pthread_mutex_lock(&cWm);
    carsWaiting--;
    pthread_mutex_unlock(&cWm); //unlock critical section

    printf("CAR: Car with threadID %ld leaving the queue to load\n", self);
	fflush( stdout );

    pthread_mutex_lock(&cLm);
    carsLoaded++;
    pthread_mutex_unlock(&cLm); //unlock critical section

    sem_wait(&cLs); //place car in blocked queue of semaphore to await signal from el capitan

    pthread_mutex_lock(&cLm);
    carsLoaded--;
    pthread_mutex_unlock(&cLm); //unlock critical section

    printf("CAR: Car with threadID %ld is onboard the ferry\n", self);
	fflush( stdout );

    pthread_mutex_lock(&cSm);
    carsSailing++;
    pthread_mutex_unlock(&cSm); //unlock critical section

    sem_wait(&cSs);

    pthread_mutex_lock(&cSm);
    carsSailing--;
    pthread_mutex_unlock(&cSm); //unlock critical section

    printf("CAR: Car with threadID %ld is now unloading\n", self);
	fflush( stdout );

    pthread_mutex_lock(&cUm);
    carsUnloaded++;
    pthread_mutex_unlock(&cUm); //unlock critical section

    sem_wait(&cUs);

    pthread_mutex_lock(&cUm);
    carsUnloaded--;
    pthread_mutex_unlock(&cUm); //unlock critical section

    printf("CAR: Car with threadID %ld has unloaded\n", self);
	fflush( stdout );

	sem_wait(&cEs);

    printf("CAR: Car with threadID %ld is about to exit\n", self);
	fflush( stdout );

    
}







void *Captain(void *ptr)
{	
	printf("CAPTAIN: Captain thread started\n");
	fflush( stdout );

	for(int k = 0; k < 5; k++){
	int counter = 0;
	int trucksToLoad = 0;
	int carsToLoad = 0;
	int trucksOnTime = 0;
	int carsOnTime = 0;
	int capacity = 6; //Number of spaces in ferry
	int restOfTrucksOnTime = 0;
	
	//might not need this
	//pthread_t self;
    //self = pthread_self();
	
	
	while ((carsWaiting + trucksWaiting) < 8) //Wait until 8 vehicles are ready to load
	{
		usleep(1000000);
	}
	trucksOnTime = trucksWaiting; //Save amount of trucks and cars that were on time
	carsOnTime = carsWaiting;
	
	if (trucksOnTime > 2)  //Can have at most 2 trucks
	{
		trucksToLoad = 2;
		restOfTrucksOnTime = trucksOnTime - 2;
	}
	else 
	{
		trucksToLoad = trucksOnTime;
	}
	capacity -= 2*trucksToLoad;
	carsToLoad = capacity; //Trucks take 2 spaces, remaining capacity is how many cars are needed
	if (carsToLoad > carsOnTime) //Not enough cars to fill ferry
	{
		capacity -= carsOnTime;
	}
	else //More cars on time than were needed to fill ferry
	{
		capacity -= carsToLoad;
	}
	while (capacity > 0)     
	{
		if ((trucksWaiting > restOfTrucksOnTime) && (trucksToLoad < 2) && capacity != 1) //If there are extra trucks waiting and there is space on the ferry
		{
			trucksToLoad++;           //Loading another truck
			restOfTrucksOnTime--;     //Moving a truck that was on time into the ferry
			capacity -= 2;            //The extra truck takes 2 spaces in the ferry
			carsToLoad -= 2;          //2 less cars are needed to fill the ferry
			continue;
		}
		if (carsWaiting > carsOnTime) //If more cars have entered the queue since the captain "woke up"
		{
			carsToLoad++;	//Add another car to the ferry
			capacity--;     //Car takes up one space in the ferry
		}
	}
	if(k == 4){
		sailing = 0;
	}	
	

	for (int i=0; i<trucksToLoad; i++)
	{
		printf("CAPTAIN: Truck selected for loading\n");
		fflush( stdout );
		sem_post(&tWs);
		printf("CAPTAIN: Captain knows truck is loaded\n");
		fflush( stdout );
	}
	for (int j=0; j<carsToLoad; j++)
	{
		printf("CAPTAIN: Car selected for loading\n");
		fflush( stdout );
		sem_post(&cWs);
		printf("CAPTAIN: Captain knows car is loaded\n");
		fflush( stdout );
	}
	printf("CAPTAIN: Ferry is full, starting to sail\n");
	fflush( stdout );
	
	for (int i=0; i<trucksToLoad; i++)
	{
		sem_post(&tLs);
	}
	for (int j=0; j<carsToLoad; j++)
	{
		sem_post(&cLs);
	}

	usleep(2000000);

	printf("CAPTAIN: Ferry has reached destination port\n");
	fflush( stdout );
	
	for (int i=0; i<trucksToLoad; i++)
	{
		sem_post(&tSs);
	}
	for (int j=0; j<carsToLoad; j++)
	{
		sem_post(&cSs);
	}
	
	for (int i=0; i<trucksToLoad; i++)
	{
		counter = truckUnloaded;
		sem_post(&tUs);
		printf("CAPTAIN: Captain knows a truck has unloaded from the ferry\n");
	}/////////////////////////////////////////////////
	for (int j=0; j<carsToLoad; j++)
	{
		counter = carsUnloaded;
		sem_post(&cUs);
		printf("CAPTAIN: Captain knows a car has unloaded from the ferry\n");
		fflush( stdout );
	}
	
	
	for (int i=0; i<trucksToLoad; i++)
	{	
		sem_post(&tEs);
		printf("CAPTAIN: Captain sees a truck is leaving the ferry terminal\n");
		fflush( stdout );
	}
	for (int j=0; j<carsToLoad; j++)
	{
		sem_post(&cEs);
		printf("CAPTAIN: Captain sees a car leaving the ferry terminal\n");
		fflush( stdout );
	}
	//return to ferry dock
	usleep(2000000);
	}
	


	
}













