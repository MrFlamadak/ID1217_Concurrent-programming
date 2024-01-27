/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
             the total sum from partial sums computed by Workers
             and prints the total sum to the standard output

   usage under Linux:
     gcc matrixSum.c -lpthread
     a.out size numWorkers

*/
#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

pthread_mutex_t barrier;  /* mutex lock for the barrier */
pthread_mutex_t mutex;
pthread_cond_t go;        /* condition variable for leaving */
int numWorkers;           /* number of workers */ 
int numArrived = 0;       /* number who have arrived */

/* a reusable counter barrier */
void Barrier() {
  pthread_mutex_lock(&barrier);
  numArrived++;
  if (numArrived == numWorkers) {
    numArrived = 0;
    pthread_cond_broadcast(&go);
  } else
    pthread_cond_wait(&go, &barrier);
  pthread_mutex_unlock(&barrier);
}

/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double start_time, end_time; /* start and end times */
int size, stripSize;  /* assume size is multiple of numWorkers */
long sums[MAXWORKERS]; /* partial sums b) remove these */
int mins[MAXWORKERS]; /* partial mins b) remove these */
int maxs[MAXWORKERS]; /* partial maxs b) remove these */
int minPositions[MAXWORKERS][2]; /* partial maxs b) remove these */
int maxPositions[MAXWORKERS][2]; /* partial maxs b) remove these */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

void *Worker(void *);
void *SumWorker(void *);
int sum = 0;

struct matrixExtrema {
    int Value;
    int x;
    int y;
};

/* Each worker sums the values in one strip.
  Then returns this value to the main thread */
void *SumWorker(void *arg)
{
  long myid = (long) arg;
  int total, i, j, first, last;
  /* determine first and last rows of my strip */
  first = myid*stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);
  //printf("Thread %d sums from (%d, %d) to (%d, %d) and", myid, first, 0, last, size);

  /* sum values in my strip */
  total = 0;
  for (i = first; i <= last; i++){
    for (j = 0; j < size; j++){
      total += matrix[i][j];
    }
  }
    pthread_mutex_lock(&mutex);
    sum += total;
    pthread_mutex_unlock(&mutex);
    //printf("sum=%d so %d was added\n", sum, total);
  return NULL;
}

/*the idea is to join the threads and retrieve value. How?
 I make 3 new thread functions which return local sum, min and max respecitvely, then I compare in main thread 
*/


/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg) {
  long myid = (long) arg;
  int total, i, j, first, last;
#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif


  /* determine first and last rows of my strip */
  first = myid*stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

  /* sum values in my strip */
  //printf("Thread %d sums from (%d, %d) to (%d, %d)\n", myid, first, 0, last, size);
  total = 0;
  for (i = first; i <= last; i++)
    for (j = 0; j < size; j++){
      total += matrix[i][j];
    }
  sums[myid] = total;
      //printf("%d was added\n", total);
  Barrier();
  if (myid == 0) {
    total = 0;
    for (i = 0; i < numWorkers; i++)
    {
      total += sums[i];
    }
    /* get end time */
    end_time = read_timer();
    /* print results */
    printf("The total is %d\n", total);
    printf("The execution time is %g sec\n", end_time - start_time);
  }
}


  void matrixsumWithBarrier()
  {
  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* initialize mutex and condition variable */
  pthread_mutex_init(&barrier, NULL);
  pthread_cond_init(&go, NULL);

    start_time = read_timer();
    for (l = 0; l < numWorkers; l++)
    {
      pthread_create(&workerid[l], &attr, Worker, (void *) l);
    }
    pthread_exit(NULL);
    pthread_mutex_destroy(&barrier);
  }
  void matrixSumWithMutex()
  {
    int i, j;
    long l; /* use long in case of a 64-bit system */
    pthread_attr_t attr;
    pthread_t workerid[MAXWORKERS];
    /* set global thread attributes */
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    /* initialize mutex and condition variable */
    pthread_mutex_init(&mutex, NULL);

    /* do the parallel work: create the workers */
    start_time = read_timer();
    for (l = 0; l < numWorkers; l++)
    {
      pthread_create(&workerid[l], &attr, SumWorker, (void *) l);
    }
    for (l = 0; l < numWorkers; l++)
    {
      pthread_join(workerid[l], NULL);
    }
    end_time = read_timer();

    printf("The total is %d\n", sum);
    //printf("The Min value is %d and its position is (%d, %d)\n", min, minPosition[0], minPosition[1]);
    //printf("The Max value is %d and its position is (%d, %d)\n", max, maxPosition[0], maxPosition[1]);
    printf("The execution time is %g sec\n", end_time - start_time);
    pthread_mutex_destroy(&mutex);
} 

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  stripSize = size/numWorkers;

  /* initialize the matrix */
  for (int i = 0; i < size; i++) {
	  for (int j = 0; j < size; j++) {
          matrix[i][j] = rand() % 99;//rand()%99;
	  }
  }
 //   for (int i = 0; i < size; i++) {
//	  printf("[ ");
//	  for (int j = 0; j < size; j++) {
//	    printf(" %d", matrix[i][j]);
//	  }
//	  printf(" ]\n");
 // }
  /* do the parallel work: create the workers */
  matrixSumWithMutex();
  matrixsumWithBarrier();
  return 0;  
}

