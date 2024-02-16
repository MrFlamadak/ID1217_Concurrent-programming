/*
The Bear and Honeybees Problem (multiple producers - one consumer) (20 points)

Given are n honeybees and a hungry bear. They share a pot of honey. The pot is initially empty;
its capacity is H portions of honey. The bear sleeps until the pot is full, eats all the honey,
and goes back to sleep. Each bee repeatedly gathers one portion of honey and puts it in the pot;
the bee who fills the pot awakens the bear.
Develop and implement a multithreaded program to simulate the actions of the bear and honeybees.
Represent the bear and honeybees as concurrent threads (i.e., a "bear" thread and an array of "honeybee" threads)
and the pot as a critical shared resource that can be accessed by at most one thread at a time. 
Use only semaphores for synchronization. Your program should print a trace of interesting simulation events.
Is your solution fair (w.r.t. honeybees)? Explain when presenting homework.

*/
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#define DEFAULT_NUMBER_OF_BEES 10
#define MAX_NUMBER_OF_BEES 30

sem_t potIsNotFull;
sem_t potIsFull;
sem_t potAccess;

int h = 10;
int pot = 0;


void* Bear(void*);
void* Bee(void*);

void* Bear(void* arg)
{
    //wait untill pot is full
    sem_wait(&potIsFull);
    sem_wait(&potAccess);
    printf("Bear is eating from the honey pot");
    usleep(100000);
    pot = 0;
    printf("Bear is sleeping");
    sem_post(&potAccess);
    sem_post(&potIsNotFull);
   
    //eat all honey in pot
}

void* Bee(void* arg)
{
    //fill pot
    //if pot full
    //wake bear

}

int main(int argc, char const *argv[])
{
    long i;
    int numberOfBees = (argc > 1) ? atoi(argv[1]) : DEFAULT_NUMBER_OF_BEES;
    if (numberOfBees > 30) {
        numberOfBees = MAX_NUMBER_OF_BEES;
    }
    pthread_t bee[numberOfBees];
    pthread_t bear;
    sem_init(potAccess, 0, 1);
    sem_init(potIsNotFull, 0, 1);
    sem_init(potIsFull, 0, 0);
    pthread_create(&bear, NULL, Bear, NULL);
    for (i = 0; i < numberOfBees; i++)
    {
        pthread_create(&bee[i], NULL, Bee, (void*) i);
    }
    for (i = 0; i < numberOfBees; i++)
    {
        pthread_join(bee[i], NULL);
    }
    pthread_join(bear, NULL);

    sem_destroy(&potAccess);
    sem_destroy(&potIsFull);
    sem_destroy(%potIsNotFull);

    return 0;
}
