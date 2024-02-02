/*
 ComputePi is a program that utilizes adaptive quadrature to compute pi. 
 This is done by applying the quadrature formula on the
 upper-right quadrant of the unit circle function f(x) = sqrt(1-x^2).
*/
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#define MAXTHREADS 10
#define DEFAULTERRORMARGIN 0.1e-4

double pi;
double errorMargin;
long numThreads;
double partitionsize;
double start_time;
double end_time;

pthread_mutex_t areaMutex;

void *PiWorker(void *);

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



double f(double x) {
    return  sqrt((1-x)*(1+x));
}

double AdaptiveQuadrature(double l, double r, double fl, double fr, double area) {
    double m = (l+r)/2;
    double fm = f(m);

    double leftarea = ((fl + fm)*(m - l))/2;
    double rightarea = ((fm + fr)*(r - m))/2;
    if(fabs((leftarea+rightarea) - area) > errorMargin)
    {
        leftarea = AdaptiveQuadrature(l, m, fl, fm, leftarea);
        rightarea = AdaptiveQuadrature(m, r, fm, fr, rightarea);
    }
    return (double) (leftarea + rightarea);
}

void *PiWorker(void *arg)
{
    long myId = (long) arg;
    double a = myId*partitionsize;
    double b = a + partitionsize;
    double fa = f(a);
    double fb = f(b);
    double areaPartition = ((b - a)*(fb + fa))/2;
    //printf("myId:%lf\n", myId);
    //printf("a:%lf\n", a);
    //printf("b:%lf\n", b);
    //printf("fa:%lf\n", fa);
    //printf("fb:%lf\n", fb);
    //printf("areaPart:%lf\n", areaPartition);
    
    double piPartition = AdaptiveQuadrature(a, b, fa, fb, areaPartition)*4;

    pthread_mutex_lock(&areaMutex);
    pi = pi + piPartition;
    pthread_mutex_unlock(&areaMutex);

    return NULL;
}

int main(int argc, char* argv[]) {
    long id;
    /* read command line args if any */
    numThreads = (argc > 1)? atol(argv[1]) : MAXTHREADS;
    errorMargin = (argc > 2)? atof(argv[2]) : DEFAULTERRORMARGIN;
    if (numThreads > MAXTHREADS) numThreads = MAXTHREADS;
    partitionsize = 1.0/numThreads;
    errorMargin = errorMargin*numThreads;
    pthread_t worker[numThreads];
    pthread_mutex_init(&areaMutex, NULL);

    start_time = read_timer();
    for(long id = 0; id < numThreads; id++)
    {
        pthread_create(&worker[id], NULL, PiWorker, (void *) id);
    }
    for(id = 0; id < numThreads; id++)
    {
        pthread_join(worker[id], NULL);
    }
    end_time = read_timer();
    pthread_mutex_destroy(&areaMutex);
    printf("pi = %.10lf\n", pi);
    printf("error margin = %.10lf\n", errorMargin);
    printf("The execution time is %g sec\n", end_time - start_time);
    return 0;
}
