
/*
To compile:
gcc -fopenmp -o parallel_bucket parallel_bucket.c -lm

To run:
./parallel_bucket

*/

#include<omp.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}

void bucket_sort(int*,int,int);
void ParallelSelectionSort(int*, int,int);
void shuffle(int *, int);
void printArr(int *, int);
bool verify(int *, int);
void parallelAssign(int *, int *, int);


//idea of parallelized bucket sort
//First we will divide the dataset into N partitions,
//there are multiple ways to do this but we'll approach it this way
/*
1. create p x b subarrays
    where p is # of partitions and b is size of each list
    b is going to be n/b and taking the ceiling value of such
    note each subarray will contain an interval of data
2. we multithread adding to the array since they are responsible for [0,b],[b,b*2],[b*3,b*4] datasets
    they will be added in random order so it needs to be sorted in the next step
3. once we have added to the dataset, we will perform a sorting method on this dataset
    for proper analysis we'll be doing bubble sort
4.Then once each bucket is sorted we can do a simple pop from each bucket, cause we can compare bucket values and pop the minimum value
into the main list, overriding it
*/
void bucket_sort(int * arr, int size, int thread_count){
    //this is just sequential bucket sort
    if (thread_count == 1){
        ParallelSelectionSort(arr,size,thread_count);
    }

    int partitions = thread_count;

    //creating sizes for all partitions, except laat one which will hold remainder
    //realistically we are assuming large value such that size > partitions^2 otherwise
    //overhead cost will make the sort inefficient
    int b = size/partitions;
    int extra = b + size%partitions;

    //assigning buckets
    int** bucket = (int**)malloc(partitions * sizeof(int*));
    for (int i = 0; i < partitions-1; i++){
        bucket[i] = (int*)malloc(b * sizeof(int));
    }
    //special case to be handled
    bucket[partitions-1] =  (int*)malloc(extra * sizeof(int));

    //to keep some ordered sort were appending to tail
    int * tail = (int*)malloc(partitions * sizeof(int));
    for(int i = 0; i < partitions; i++){
        tail[i] = 0;
    }

    #pragma omp parallel shared(arr,bucket,tail) num_threads(thread_count) 
    {

        
        //buckets need to be of interval [0,b] but when we are adding it
        //an all to all relation is needed such that one thread can add a data another thread is responsible for
        #pragma omp for
        for(int i = 0; i<size; i++){

            int bucketLoc = arr[i]/b; //taking floor of result

            //accounting for the extras
            if(bucketLoc == partitions){
                bucketLoc-=1;
            }
            bucket[bucketLoc][tail[bucketLoc]] = arr[i];
            tail[bucketLoc]++;
        }

        #pragma omp barrier

        //we now added stuff into buckets
        //now we will sort using selection sort
        #pragma omp for
        for(int i = 0; i < thread_count; i++){
            if(omp_get_thread_num()!= partitions-1){
                ParallelSelectionSort(bucket[omp_get_thread_num()],b,thread_count);
            }
            else{
                ParallelSelectionSort(bucket[omp_get_thread_num()],extra,thread_count);
            }
        }
    

    

        #pragma omp barrier
        
        //now that we have sorted each interval we need to add them in accordingly
        #pragma omp for
        for(int p = 0; p < partitions; p++){    
            for(int i = 0; i < size; i++){
                //normal case
                if(i/b != partitions){
                    arr[i] = bucket[i/b][i%b];
                }
                //end/edge case
                else{
                    arr[i] = bucket[i/b-1][b+(i%b)];
                }
            }
        }

        #pragma omp barrier

    }


}



//Custom struct to compare value and create a reduction area
struct Compare { int val; int index; };
#pragma omp declare reduction(maximum : struct Compare : omp_out = omp_in.val > omp_out.val ? omp_in : omp_out)

void ParallelSelectionSort(int * arr, int size, int thread_count){

    //starting at end (max value) we'll call this pointer 1
    for (int i = size-1; i >= 0; i--)
    {
        //declare max value here
        struct Compare max;
        max.val = arr[i];
        max.index = i;

        //this is where we use the 2nd pointer and scan rest of list
        //this will use OMP to multithread it so there are more than 1 "pointer" per-se
        //but because we reduce the results we get from each result we can combine them and use the custom
        //declaration reduction above to compile all the threaded results into 1 variable with that clause
        
        //basic selection sort with parallelization reduction
        #pragma omp parallel for reduction(maximum:max) num_threads(thread_count)
        for (int j = i-1; j >= 0; j--)
        {
            if (arr[j] > max.val)
            {
                max.val = arr[j];
                max.index = j;
            }
        }
        int tmp = arr[i];
        arr[i] = max.val;
        arr[max.index] = tmp;
    }
}

void parallelAssign(int * arr, int *arr2, int size){
    #pragma omp parallel for shared(arr,arr2)
    for(int i = 0; i < size; i++){
        arr2[i] = arr[i];
    }
}

//simple shuffle function
void shuffle(int * arr, int size){
    srand(time(NULL));
    for(int i = 0; i < size; i++){
        int randNum = rand()%size;
        int temp = arr[i];
        arr[i] = arr[randNum];
        arr[randNum] = temp;
    }
}

//debugging purposes
void printArr(int * arr, int size){
    for(int i = 0; i < size;i++){
        printf("[%i]",arr[i]);
    }
    printf("\n");
}

//simple verification
bool verify(int * arr, int size){
    for(int i = 0; i < size-1; i++){
        if(arr[i] > arr[i+1])return false;
    }
    return true;
}

int main(){
/*


    CONFIGURATIONS TO BE DONE HERE

    Test size ->
        will test the bucket sort algorithm at varying sizes
    
    test processor count ->
        will test with increasing thread count up until the max thread count possible (declared above)

    test bucketSort/SelectionSort ->
        will display which algorithm we are using so we do not flood results
        if all are turned on, beware it may take a minute or longer depending on processing time

    MAX_size ->
        maximum size for the testing the size testing portion of the analysis

    size -> 
        the size of the data for fixed

    max_thread_count ->
        Maximum threads allowed, should not be set higher than your computer thread count

*/
    
    //Which test to run
    bool testSize = false;
    bool testProcessorCount = true;

    //Algorithm
    bool testBucketSort = false;
    bool testSelectionSort = true;
    
    //configurations
    int MAX_SIZE = 1000000;
    int size = 100000;
    int MAX_THREAD_COUNT = 8;


    //for sakes and purposes we'll we'll be creating a list of numbers
    //0->n
    //we are guaranteeing uniqueness, if we did not then we would have to include
    //a comparator for what happens when 2 values are the same
    int * arr;
    int * randArr; //for faster shuffling during processor count testing
    
    
    double start,finish,elapsed;

    //TESTING ON SIZE
    if(testSize && testBucketSort){
        for(int i = 1000; i <= MAX_SIZE; i*=10){

            //allocating size and shuffling
            arr=(int*)malloc(i*sizeof(int));
            for(int j = 0; j<i; j++){arr[j] = j;}
            shuffle(arr,i);
            
            //testing
            GET_TIME(start);
            bucket_sort(arr,i,MAX_THREAD_COUNT);
            GET_TIME(finish);
            elapsed = finish-start;
            free(arr);
            printf("Parallel Bucket Sort + Parallel Selection Sort\n");
            printf("Size vs Time\n");
            printf("Array Size: %i\n",i);
            printf("Thread Count: %i\n", MAX_THREAD_COUNT);
            printf("Elapsed Time: %f seconds\n", finish-start);
            printf("\n\n");
        }
    }

    if(testProcessorCount && testBucketSort){
        //allocating size and shuffling
        arr=(int*)malloc(size*sizeof(int));
        for(int j = 0; j<size; j++){arr[j] = j;}
        shuffle(arr,size);

        for(int i = 1; i <= MAX_THREAD_COUNT; i++){
            randArr=(int*)malloc(size*sizeof(int));
            parallelAssign(arr,randArr,size);

            //testing
            GET_TIME(start);
            bucket_sort(randArr,size,i);
            GET_TIME(finish);
            elapsed = finish-start;
            free(randArr);

            printf("Parallel Bucket Sort + Parallel Selection Sort\n");
            printf("Processor count vs Time\n");
            printf("Array Size: %i\n",size);
            printf("Thread Count: %i\n", i);
            printf("Elapsed Time: %f seconds\n", finish-start);
            printf("\n\n");
        }
    }

    if(testSize && testSelectionSort){
        for(int i = 1000; i <= MAX_SIZE; i*=10){

            //allocating size and shuffling
            arr=(int*)malloc(i*sizeof(int));
            for(int j = 0; j<i; j++){arr[j] = j;}
            shuffle(arr,i);
            
            //testing
            GET_TIME(start);
            ParallelSelectionSort(arr,i,MAX_THREAD_COUNT);
            GET_TIME(finish);
            elapsed = finish-start;
            free(arr);

            printf("Parallel Selection Sort\n");
            printf("Size vs Time\n");
            printf("Array Size: %i\n",i);
            printf("Thread Count: %i\n", MAX_THREAD_COUNT);
            printf("Elapsed Time: %f seconds\n", finish-start);
            printf("\n\n");
        }
    }


    if(testProcessorCount && testSelectionSort){
        //allocating size and shuffling
        arr=(int*)malloc(size*sizeof(int));
        for(int j = 0; j<size; j++){arr[j] = j;}
        shuffle(arr,size);

        for(int i = 1; i <= MAX_THREAD_COUNT; i++){
            randArr=(int*)malloc(size*sizeof(int));
            parallelAssign(arr,randArr,size);
            
            //testing
            GET_TIME(start);
            ParallelSelectionSort(randArr,size,i);
            GET_TIME(finish);
            elapsed = finish-start;
            free(randArr);

            printf("Parallel Selection Sort\n");
            printf("Processor count vs Time\n");
            printf("Array Size: %i\n",size);
            printf("Thread Count: %i\n", i);
            printf("Elapsed Time: %f seconds\n", finish-start);
            printf("\n\n");
        }
    }




    return 0;
}