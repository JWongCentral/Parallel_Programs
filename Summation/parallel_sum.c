
#include<omp.h>
#include<stdio.h>
#include<stdlib.h>

#define MAX_THREAD_COUNT 8
#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}

void parallel_sum(int,int);

void parallel_sum (int val, int thread_count){

    long int total = 0;
	#pragma omp parallel for reduction(+:total) num_threads(thread_count) 
    for(int i = 0; i <= val; i++){
        total += i;
    }

    printf("The summation of %i is %li\n", val, total);
    
}

int main(){
    double start, finish, elapsed;
    //we'll be measuring in powers of 10 just to make runtime easier
    int size = 100000000;

    for(int thread_count = 1; thread_count <= MAX_THREAD_COUNT; thread_count++){
        printf("Using %i number of threads\n",thread_count);
        
        GET_TIME(start);
        parallel_sum(size,thread_count);
        GET_TIME(finish);
        elapsed = finish-start;
        printf("The code to be timed took %g seconds\n\n", elapsed);
    }
    
   
    
}