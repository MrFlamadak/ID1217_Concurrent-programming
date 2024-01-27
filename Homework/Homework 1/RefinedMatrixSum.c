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
pthread_mutex_t sumMutex;
pthread_mutex_t minmaxMutex;
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
long sums[MAXWORKERS]; /* partial sums */
int mins[MAXWORKERS]; /* partial mins */
int maxs[MAXWORKERS]; /* partial maxs */
int minPositions[MAXWORKERS][2]; /* partial maxs */
int maxPositions[MAXWORKERS][2]; /* partial mins */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

//b)
int sum = 0;
int min = 100;
int max = 0; 

void *Worker(void *);
void *MutexWorker(void *);

struct matrixExtrema
 {
    int max;
    int maxX;
    int maxY;
    int min;
    int minX;
    int minY;
};
struct matrixExtrema extrema;

/* Each worker sums the values in one strip.
  Then returns this value to the main thread */
void *MutexWorker(void *arg)
{
  long myid = (long) arg;
  int total, i, j, first, last;
  /* determine first and last rows of my strip */
  first = myid*stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);
  int myMax = 0;
  int myMin = 100;

  /* sum values in my strip */
  total = 0;
  for (i = first; i <= last; i++){
    for (j = 0; j < size; j++){
      total += matrix[i][j];
      //please make into function later
      /***********************************/
      if(myMin > matrix[i][j] )
      {
        myMin = matrix[i][j];
        if(myMin < extrema.min)
        {
          pthread_mutex_lock(&minmaxMutex);
          if(myMin < extrema.min)
          {
            extrema.min = myMin;
            extrema.minX = i;
            extrema.minY = j;
          }
          else
          {
            myMin = extrema.min;
          }
          pthread_mutex_unlock(&minmaxMutex);
        }
      }
      if(myMax < matrix[i][j] )
      {
        myMax = matrix[i][j];
        if(myMax > extrema.max)
        {
          pthread_mutex_lock(&minmaxMutex);
          if(myMax > extrema.max)
          {
            extrema.max = myMax;
            extrema.maxX = i;
            extrema.maxY = j;
          }
          else
          {
            myMax = extrema.max;
          }
          pthread_mutex_unlock(&minmaxMutex);
        }
      }
      /************************************/
    }
  }
    pthread_mutex_lock(&sumMutex);
    sum += total;
    pthread_mutex_unlock(&sumMutex);

  return NULL;
}

/*the idea is to join the threads and retrieve value. How?
 I make 3 new thread functions which return local sum, min and max respecitvely, then I compare in main thread 
*/


/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
  void *Worker(void *arg)
  {
  long myid = (long) arg;
  int total, i, j, first, last;
  //added a) assignment
  /******************/
  int min = 100;
  int max = 0;
  int minPosition[2];
  int maxPosition[2];
  /******************/
#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

  /* determine first and last rows of my strip */
  first = myid*stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

  /* sum values in my strip */
  total = 0;
  for (i = first; i <= last; i++)
    for (j = 0; j < size; j++){
      total += matrix[i][j];
      //added a) assignment
      /******************/
      if(matrix[i][j] > max)
      {
        max = matrix[i][j];
        maxPosition[0] = i;
        maxPosition[1] = j;
      }
      if(matrix[i][j] < min)
      {
        min = matrix[i][j];
        minPosition[0] = i;
        minPosition[1] = j;
      }
      /******************/
    }
  sums[myid] = total;
  mins[myid] = min;
  maxs[myid] = max;
  minPositions[myid][0] = minPosition[0];
  minPositions[myid][1] = minPosition[1];
  maxPositions[myid][0] = maxPosition[0];
  maxPositions[myid][1] = maxPosition[1];
  Barrier();
  if (myid == 0) {
    total = 0;
    min = 100;
    max = 0;
    for (i = 0; i < numWorkers; i++)
    {
      total += sums[i];

      if(mins[i] < min)
      {
        min = mins[i];
        minPosition[0] = minPositions[i][0];
        minPosition[1] = minPositions[i][1];
      }
      if(maxs[i] > max)
      {
        max = maxs[i];
        maxPosition[0] = maxPositions[i][0];
        maxPosition[1] = maxPositions[i][1];
      }
    }
    /* get end time */
    end_time = read_timer();
    /* print results */
    printf("The total is %d\n", total);
    printf("The Min value is %d and its position is (%d, %d)\n", min, minPosition[0], minPosition[1]);
    printf("The Max value is %d and its position is (%d, %d)\n", max, maxPosition[0], maxPosition[1]);
    printf("The execution time is %g sec\n", end_time - start_time);
  }
}


  void matrixOperationsWithBarrier()
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
  void matrixOperationsWithMutex()
  {
    extrema.max = 0;
    extrema.min = 100; 
    int i, j;
    long l; /* use long in case of a 64-bit system */
    pthread_attr_t attr;
    pthread_t workerid[MAXWORKERS];
    /* set global thread attributes */
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    /* initialize mutex and condition variable */
    pthread_mutex_init(&sumMutex, NULL);
    pthread_mutex_init(&minmaxMutex, NULL);

    /* do the parallel work: create the workers */
    start_time = read_timer();
    for (l = 0; l < numWorkers; l++)
    {
      pthread_create(&workerid[l], &attr, MutexWorker, (void *) l);
    }
    for (l = 0; l < numWorkers; l++)
    {
      pthread_join(workerid[l], NULL);
    }
    end_time = read_timer();

    printf("The total is %d\n", sum);
    printf("The Min value is %d and its position is (%d, %d)\n", extrema.min, extrema.minX, extrema.minY);
    printf("The Max value is %d and its position is (%d, %d)\n", extrema.max, extrema.maxX, extrema.maxY);
    printf("The execution time is %g sec\n", end_time - start_time);
    pthread_mutex_destroy(&sumMutex);
    pthread_mutex_destroy(&minmaxMutex);
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
          matrix[i][j] = rand() % 58;//rand()%99;
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
  matrixOperationsWithMutex();
  matrixOperationsWithBarrier();
  return 0;  
}

