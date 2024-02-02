#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>

#define MAX_THREADS 16
#define EPSILON 1e-10

// Function to compute f(x) = 4 / (1 + x^2)
double f(double x) {
    return 4.0 / (1.0 + x * x);
}

// Struct to hold arguments for each thread
typedef struct {
    double a;
    double b;
    double result;
} ThreadData;

// Function to be executed by each thread
void* computePi(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    data->result = (data->b - data->a) * (f(data->a) + f(data->b)) / 2.0;
    return NULL;
}

// Recursive function for parallel computation of pi
double computePiRecursive(double a, double b, int depth, int max_depth) {
    if (depth >= max_depth || fabs(b - a) < EPSILON) {
        return (b - a) * (f(a) + f(b)) / 2.0;
    }

    double mid = (a + b) / 2.0;

    // Create thread data for the left and right subintervals
    ThreadData leftData = {a, mid, 0.0};
    ThreadData rightData = {mid, b, 0.0};

    // Create threads for the left and right subintervals
    pthread_t leftThread, rightThread;

    pthread_create(&leftThread, NULL, computePi, (void*)&leftData);
    pthread_create(&rightThread, NULL, computePi, (void*)&rightData);

    // Wait for threads to finish
    pthread_join(leftThread, NULL);
    pthread_join(rightThread, NULL);

    return leftData.result + rightData.result;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_threads> <max_depth>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_threads = atoi(argv[1]);
    int max_depth = atoi(argv[2]);

    if (num_threads < 1 || num_threads > MAX_THREADS || max_depth < 1) {
        fprintf(stderr, "Invalid arguments\n");
        exit(EXIT_FAILURE);
    }

    double a = 0.0;
    double b = 1.0;
    double result = 0.0;

    // Measure the start time
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    // Compute pi using recursive parallelization
    result = computePiRecursive(a, b, 0, max_depth);

    // Measure the end time
    gettimeofday(&end_time, NULL);

    // Calculate the elapsed time in milliseconds
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0 +
                          (end_time.tv_usec - start_time.tv_usec) / 1000.0;

    printf("Result: %f\n", result);
    printf("Time taken by threads: %.2f milliseconds\n", elapsed_time);

    return 0;
}