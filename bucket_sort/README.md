# Parallel bucket sort
one of parallelization is using bucket sort since you can define the size of each bucket to reduce overhead cost of using/starting multiple threads.
Then we can run a simple bubble sort on the psuedo-sorted list then combining each individual bucket together to complete the sorted list.
Compared to other sorting algorithms like selection sort for example bucket sort performs better due to its divide and conquer tactics reducing the 
problem size to n/b where b is the number of buckets to get a runtime of about (n/b)^2 compared to selection sort (n^2) runtime.
