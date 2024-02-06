#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#define MAXIMUMLENGTH 100

//Mutexes and condition variables
pthread_mutex_t mutexforWriters = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t inputCondition = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutexforData = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t outputCondition = PTHREAD_COND_INITIALIZER;


pthread_mutex_t barrier;  /* mutex lock for the barrier */

pthread_cond_t go;        /* condition variable for leaving */
int numWorkers = 2 ;           /* number of workers */ 
int numArrived = 0;       /* number who have arrived */

//Shared variables
char string[MAXIMUMLENGTH];
bool dataReady = false;

void *Reader(void*);
void *FileWriter(void*);
void *SystemWriter(void*);

/* a reusable counter barrier */
void Barrier() {
  pthread_mutex_lock(&barrier);
  numArrived++;
  if (numArrived == numWorkers) {
    numArrived = 0;
    pthread_mutex_lock(&mutexforData);
    dataReady = false;
    pthread_mutex_unlock(&mutexforData);
    pthread_cond_signal(&outputCondition);
    pthread_cond_broadcast(&go);
  } else
  { 
    pthread_mutex_lock(&mutexforData);
    dataReady = false;
    pthread_mutex_unlock(&mutexforData);
    pthread_cond_wait(&go, &barrier);
  }
  pthread_mutex_unlock(&barrier);
}


void waitForData()
{
    pthread_mutex_lock(&mutexforData);
    while(!dataReady)
    {
        pthread_cond_wait(&inputCondition, &mutexforData);
    }
    pthread_mutex_unlock(&mutexforData);
}
void waitForWriters()
{
    pthread_mutex_lock(&mutexforWriters);
    pthread_cond_wait(&outputCondition, &mutexforWriters);
    pthread_mutex_unlock(&mutexforWriters);
}


void *Reader(void* arg) {
    while(1) {
    // System input

    fgets(string, sizeof(string), stdin);
    pthread_mutex_lock(&mutexforData);
    dataReady = true;
    pthread_cond_broadcast(&inputCondition);
    pthread_mutex_unlock(&mutexforData);
    waitForWriters();
    //memset(string, 0, sizeof(string));
    }
    pthread_exit(NULL);
}

void *FileWriter(void* arg) {
    FILE* wFile = (FILE*) arg;
    while(1)
    {
        waitForData();
        fputs(string, wFile);
        fflush(wFile);
        Barrier();
    }
}

void *SystemWriter(void* arg) {
    while(1)
    {
        waitForData();
        fputs(string, stdout);
        Barrier();
    }
}

int main(int argc, char const *argv[])
{
    //initalizing threads, mutexes and variables
    pthread_t fileWriter;
    pthread_t reader;
    pthread_t systemWriter;

    FILE* wFile;
    wFile = fopen(argv[1], "w");
    if(wFile == NULL)
    {
        printf("File couldn't be opened, program terminating.");
        exit(0);
    }
    //creating threads
    pthread_create(&reader, NULL, Reader, NULL);
    pthread_create(&systemWriter, NULL, SystemWriter, NULL);
    pthread_create(&fileWriter, NULL, FileWriter, wFile);

    
    pthread_join(reader, NULL);
    pthread_join(fileWriter, NULL);
    pthread_join(systemWriter, NULL);


    //closing/destroying resources
    fclose(wFile);
    pthread_mutex_destroy(&mutexforData);
    pthread_mutex_destroy(&mutexforWriters);
    pthread_cond_destroy(&inputCondition);
    pthread_cond_destroy(&outputCondition);

    
    return 0;
}
