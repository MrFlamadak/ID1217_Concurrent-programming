/*
1. The Hungry Birds Problem  (one producer - multiple consumers) (20 points)

Given are n child birds and one parent bird. The child birds eat out of a common dish that initially contains W worms.
Each child bird repeatedly takes a worm, eats it, sleeps for a while, takes another worm, and so on.
When the dish is empty, the child bird, who discovers the empty dish, chirps loudly to awaken the parent bird.
The parent bird flies off, gathers W more worms, puts them in the dish, and then waits for the dish to be empty again.
This pattern repeats forever.
Develop and implement a multithreaded program to simulate the actions of the birds.
Represent the birds as concurrent threads (i.e., an array of "babyBird" threads and a "parentBird" thread)
and the dish as a critical shared resource that can be accessed by at most one bird at a time.
Use only semaphores for synchronization. Your program should print a trace of interesting simulation events.
Is your solution fair? Explain when presenting homework.

*/
// UNFAIR SOLUTION, SOME BIRDS MAY STARVE
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#define DEFAULT_NUMBER_OF_BIRDS 10;
#define MAX_NUMBER_OF_BIRDS 500;

void* BabyBird(void* arg);
void* ParentBird(void* arg);
int dish = 10;
int numberOfBirds;
int wormsToFetch = 10;

int globalseed;

sem_t dishAcess;
sem_t emptyDish;
sem_t notEmptyDish;
sem_t yourTurn;


void* BabyBird(void* arg)
{
    long rank = (long) arg;
    unsigned int mySeed =  rank ^ globalseed;
    int mysleep = 0;
    
    while(1) {
        mysleep = rand_r(&mySeed) % 200000 + 20000;
        usleep(mysleep);
        sem_wait(&yourTurn);
        sem_wait(&dishAcess);
        if (dish > 0) {
            printf("bird %ld is eating a worm\n", rank);
            usleep(40000);
            dish--;
            printf("%d worms left\n", dish);
        }
        else
        {
            printf("bird %ld is chrping for food\n", rank );
            sem_post(&dishAcess);
            sem_post(&emptyDish);
            sem_wait(&notEmptyDish);
            sem_wait(&dishAcess);
            printf("bird %ld is eating a worm\n", rank);
            dish--;
            printf("%d worms left\n", dish);
        }
        printf("bird %ld is sleeping\n", rank);
        sem_post(&dishAcess);
        sem_post(&yourTurn);
        //printf("%d", rand_r(&mySeed) % 100000);
    }
}


void* ParentBird(void* arg)
{
    long rank = (long) arg;
    unsigned int mySeed = rank ^ globalseed;
    while(1)
    {   
        sem_wait(&emptyDish);
        printf("parentbird fetches %d worms and fills dish\n", wormsToFetch);
        usleep((rand_r(&mySeed) % 100000) + 20000);
        sem_wait(&dishAcess);
        dish = wormsToFetch;
        sem_post(&dishAcess);
        sem_post(&notEmptyDish);
        printf("parentbird sleeps\n");
    }

}

int main(int argc, char const *argv[])
{
    numberOfBirds = (argc > 1) ? atoi(argv[1]) : DEFAULT_NUMBER_OF_BIRDS;
    if(numberOfBirds > 500)
        numberOfBirds = MAX_NUMBER_OF_BIRDS;
    globalseed = time(0);
    pthread_t parent;
    pthread_t child[numberOfBirds];
    sem_init(&dishAcess, 0, 1);
    sem_init(&yourTurn, 0, 1);
    sem_init(&notEmptyDish, 0, 0);
    sem_init(&emptyDish, 0, 0);
    pthread_create(&parent, NULL, ParentBird, NULL);
    for(long i = 0; i < numberOfBirds; i++)
    {
        pthread_create(&child[i], NULL, BabyBird, (void *) i);
    }
    for(long i = 0; i < numberOfBirds; i++)
    {
        pthread_join(child[i], NULL);
    }
    pthread_join(parent, NULL);
    sem_destroy(&dishAcess);
    sem_destroy(&yourTurn);
    sem_destroy(&emptyDish);
    sem_destroy(&notEmptyDish);

    return 0;
}



