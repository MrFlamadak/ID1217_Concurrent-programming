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

//Shared variables
char string[MAXIMUMLENGTH];
bool dataReady = false;
int writersFinished = 0;

void *Reader(void*);
void *FileWriter(void*);
void *SystemWriter(void*);

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
    while(writersFinished < 2)
    {
        pthread_cond_wait(&outputCondition, &mutexforWriters);

    }
    writersFinished = 0;
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
    }
}

void *FileWriter(void* arg) {
    FILE* wFile = (FILE*) arg;
    while(1)
    {
        waitForData();
        fputs(string, wFile);
        fflush(wFile);
        pthread_mutex_lock(&mutexforWriters);
        writersFinished++;
        if(writersFinished == 2) {

            pthread_cond_broadcast(&outputCondition);
        }
        pthread_mutex_unlock(&mutexforWriters);
    }
}

void *SystemWriter(void* arg) {
    while(1)
    {
        waitForData();
        fputs(string, stdout);
        pthread_mutex_lock(&mutexforWriters);
        writersFinished++;
        if(writersFinished == 2) {
            pthread_cond_broadcast(&outputCondition);
        }
        pthread_mutex_unlock(&mutexforWriters);
        pthread_mutex_lock(&mutexforData);
        dataReady = false;
        pthread_mutex_unlock(&mutexforData);
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
