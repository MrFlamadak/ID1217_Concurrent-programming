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


double start_time;
double end_time;
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

double epsilon;
double f(double x) {
    return  sqrt((1-x)*(1+x));
}
double quadratureFormula(double l, double r, double fl, double fr, double area) {
    double m = (l+r)/2;
    double fm = f(m);

    double leftarea = ((fl + fm)*(m - l))/2;
    double rightarea = ((fm + fr)*(r - m))/2;
    if(fabs((leftarea+rightarea) - area) > epsilon)
    {
        leftarea = quadratureFormula(l, m, fl, fm, leftarea);
        rightarea = quadratureFormula(m, r, fm, fr, rightarea);
    }
    return (leftarea + rightarea);
}

int main(int argc) {
    epsilon = 1e-22;
    
    start_time = read_timer();
    double pi = quadratureFormula(0, 1, f(0), f(1), 1/2.0)*4.0;
    end_time = read_timer();
    printf("pi = %.10lf\n", pi);
    printf("error margin = %.10lf\n", epsilon);
     printf("The execution time is %g sec\n", end_time - start_time);
    return 0;
}
