#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <time.h>

void childLabour(int cseed);
void gchildLabour(pid_t parentpid);

int main(){
    int parent_seed, num_children; 
    pid_t parentpid, childpid[9];


    printf("enter the seed for the parent process\n");
    //get input from user for seed
    scanf("%d", &parent_seed);
    
    srand(parent_seed);
    //generate a random number between 5 and 9 inclusive
    num_children = rand() % 5 + 5;

    
    parentpid = getpid();
    printf("My process ID id %d\n", parentpid);

    //for loop to create children
    for(int i = 0; i < num_children; i++){
        if((childpid[i] = fork()) < 0)
        {
            printf("child spawn failed");
            abort();
        }
        else if(childpid[i] == 0){
            //all work done by child process is placed here and it is terminated at end of if
            childLabour(parent_seed + i);
            exit(0);
        }
        //this is executed by parent process
        printf("Parent %d has created a child with process ID %d\n", parentpid, childpid[i]);
    }

    for(int i = 0; i < num_children; i++){
        printf("I am the parent, I am waiting for child %d to terminate\n", childpid[i]);
        /////////////////////need to probably replace NULL
        waitpid(childpid[i],NULL, 0); //returns childpid if child process has terminated
        if(i+1 < num_children){ //if not last child
            printf("I am process %d. My child %d is dead\n", getpid(), childpid[i]);
        }
        else{ //if last child ouput this line instead
            printf("I am the parent, child %d has terminated\n", childpid[i]);
        }
    }
    sleep(5); //sleep for 5 seconds


    return 0; //should terminate the original parent process
}

void childLabour(int cseed){
    int num_gchildren;
    pid_t gchildpid[3];
    pid_t mypid = getpid();
    srand(cseed);
    printf("I am a new child, my process ID is %d, my seed id %d\n", getpid(), cseed);
    num_gchildren = rand() % 3 + 1;
    printf("I am %d, I will have %d children\n", getpid(), num_gchildren);

    for(int i = 0; i < num_gchildren; i++){
        printf("I am child %d, I am about to create a child\n", getpid());
        if((gchildpid[i] = fork()) < 0)
        {
            printf("child spawn failed\n");
            abort();
        }
        else if(gchildpid[i] == 0){
            gchildLabour(mypid);
            exit(0);
        }
        //this is executed by child process
        printf("I am child %d, I just created a child\n", getpid());
    }

    printf("I am the child %d, I have %d children, \nI am waiting for my children to terminate\n", getpid(), num_gchildren);
    for(int i = 0; i < num_gchildren; i++){
        /////////////////////////////////might need to change NULL
        waitpid(gchildpid[i],NULL,0);
        printf("I am child %d. My child %d has been waited\n",getpid(),gchildpid[i]);
    }
    printf("I am child %d, I am about to terminate\n", getpid());

    sleep(5);

    return;
}

void gchildLabour(pid_t parentpid){
    int sleeptime;

    //get time here for random number of seed
    srand(time(NULL));
    
    sleeptime = rand() % 10 + 5;
    sleep(sleeptime);
    printf("I am grandchild %d with parent %d, I am about to terminate\n", getpid(), parentpid );

    return;

}
