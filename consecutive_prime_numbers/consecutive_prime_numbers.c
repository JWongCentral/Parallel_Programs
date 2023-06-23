

/*
To compile:
gcc -fopenmp -o consecutive_prime_numbers consecutive_prime_numbers.c -lm

To run:
./consecutive_prime_numbers

*/

#include<omp.h>
#include<stdio.h>
#include<stdlib.h>

#define SIZE 1000001
#define MAX_THREAD_COUNT 8
#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}

/*
    the idea for prime numbers in this case is to create a list
    1-1,000,000
    then we will use a domain reduction algorithm which will reduce
    the list so it only contains prime numbers

    the idea goes like this, if the number we want to check is 2.
    we will send one thread to remove any numbers divisible by except for the number itself
    so it should remove 4,6,8...
    
    then we can iterate thru to the next number 3 and remove all numbers
    divisible by 3 except for itself so itll remove 6,9,12...

    it will remove it by simply setting it to a 0. if we noted that 2 and 3
    both have the number 6 it would already be a 0 so its no problem

    note: logically we only need to check numbers up until size/2 otherwise we have redundancy

    this actually ends up running in log(n) time if i recall

    then what we'll be left with is an array 1,2,3,0,5...
    so we can go through the list and add any non-zero number (excluding 1) from the list
    and add it to the prime list of numbers
    
*/
void print_array(int*,int);

void print_array(int * arr, int size){
    for(int i = 0; i < size; i++){
        printf("arr[%i] = %i\n", i, arr[i]);
    }
}
int prime_numbers(int thread_count){

    //allocating arrays
    int * num  = (int*)malloc((SIZE)*sizeof(int));
    int * prime = (int*)malloc((SIZE)*sizeof(int));
    
    //Assigning the list
    #pragma omp parallel for shared(num) num_threads(thread_count)
    for (int i = 0; i < SIZE; i++){
        num[i] = i;
        prime[i] = 0;
    }
    

    //domain reduction
    #pragma omp parallel for shared(num) num_threads(thread_count)
    for (int i = 2; i < SIZE/2; i++){
        int temp = num[i];
        for (int j = temp*2; j < SIZE; j+=temp){
            if (temp == 0) break;
            num[j] = 0;
        }
    }

    //compressing to array prime (for this step we are just running it in sequential time)
    //we are omitting 1 and 2 because they are not prime numbers
    int j = 0;
    //#pragma omp parallel for shared(j,prime) num_threads(NUM_THREADS)
    for(int i=2; i < SIZE; i++){
        if(num[i] != 0){
            prime[j++]=num[i];
        }
    }    
    


    //calculate consecutive numbers
    //in order to be consecutive there must be a difference of exactly 2
    int consecutive = 0;
    #pragma omp parallel for shared(consecutive,prime) num_threads(thread_count)
    for(int i = 1; i < SIZE; i++){
        //we have searched entire list
        if(prime[i] == 0){
            i = SIZE;
        }
        if(prime[i+1]-prime[i] == 2){
            consecutive++;
        }
    }
    
    printf("The number of consecutive primes is: %i\n",consecutive);
}


int main(){
    double start, finish, elapsed;
    
    printf("Going from 1-%i consecutive primes\n\n",SIZE-1);

    for(int thread_count = 1; thread_count <= MAX_THREAD_COUNT; thread_count++){
        printf("Thread Count: %i\n", thread_count);
        GET_TIME(start);
        prime_numbers(thread_count);
        GET_TIME(finish);
        elapsed = finish-start;
        printf("Time elapsed %f Seconds\n\n",elapsed);
    }

}