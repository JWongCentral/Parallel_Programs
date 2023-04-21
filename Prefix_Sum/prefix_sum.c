/* Inclusive Prefix sum */
/* Tyler Simon */

#include<omp.h>
#include<stdio.h>
#include<stdlib.h>
#include "timer.h"
#define NUM_THREADS 10
#define DEFAULT_SIZE  100000

void prefix(int*, int*, int);
void prefix_sum(int*, int*,int);


int main(int argc, char** argv){

	int *scan;
	int *x;
	int size;
	int i;

	if (argc > 1) size = atoi(argv[1]);
	else size=DEFAULT_SIZE;

	scan=(int*)calloc(size,sizeof(int)+1);
	x=(int*)malloc(size*sizeof(int));

	srand(123);


	

	for(i=0; i<size; i++){
		x[i]=rand()%size;
		printf("x[%d] = %d\n", i,x[i]);
	}
	//sequential time
	double start, finish, elapsed1,elapsed2;
	GET_TIME(start);
	prefix(scan,x,size);
	GET_TIME(finish);
	elapsed1 = finish-start;
	for(i=0; i<size; i++)
		printf("prefix[%d] = %d\n", i,scan[i]);
	
	scan=(int*)calloc(size,sizeof(int)+1);

	GET_TIME(start);
	prefix2(scan,x,size);
	GET_TIME(finish);
	elapsed2 = finish-start;

	for(i=0; i<size; i++)
		printf("prefix[%d] = %d\n", i,scan[i]);

	printf("Sequential Time:%f\n",elapsed1);
	printf("Parallel Time:%f\n",elapsed2);
	free(x);
	free(scan);

return 0;
}


void prefix(int* prefix, int* vals, int length){
	int i;
	prefix[0]=vals[0];
	for(i=1; i<length; i++){
		prefix[i] = prefix[i-1]+vals[i];
		}
}//end prefix


//parallel prefix sum
void prefix_sum(int * prefix, int* arr, int n) {
	
	//we will paralize it by dividing the problem into T subarrays where T is number of threads
	int subarray_size = n/NUM_THREADS;
	int * prefix_block = (int*)malloc((NUM_THREADS+1)*sizeof(int));
	prefix_block[0] = 0;
	
	#pragma omp parallel for num_threads(NUM_THREADS)
	for (int i = subarray_size*omp_get_thread_num(); i < subarray_size*omp_get_thread_num()+subarray_size; i++) {
		if(i%subarray_size*omp_get_thread_num()==0 && i==0){
			prefix[i] = arr[i];
		}
		else{
			prefix[i] = prefix[i-1]+arr[i];
		}
		if (subarray_size*omp_get_thread_num()+subarray_size-1){
			prefix_block[omp_get_thread_num()+1] = prefix[i];
		}
	}
	
	

	//then we calculate prefix sum of the prefix blocks
	for(int i = 1; i < NUM_THREADS; i++){
		prefix_block[i]+=prefix_block[i-1];
	}



	//then we can just run in parallel of the prefix blocks
	//1,2,3,4,5,6,7,8,9,10
	//1,3,6,10,15,21,28,36,45,55
	#pragma omp parallel for num_threads(NUM_THREADS)
	for (int i = subarray_size*omp_get_thread_num(); i <subarray_size*omp_get_thread_num()+subarray_size ; i++){
		prefix[i] += prefix_block[i/subarray_size];
	}
	
}



