// code come from: https://blog.csdn.net/qq_36653924/article/details/127923102

#include<iostream>
#include<time.h>
#include<pthread.h>
 
#define N 500000000
#define THREAD_NUM 1
 
struct Result
{
	int *array;
	long long start;
	long long length;
	long long result;
	int index;
};
 
void rand(int *array)
{
	for(long long i=0;i<N;i++)
	{
		array[i] = rand()%100;
	}
}
 
void *func(void *result)
{
	
	if (nullptr == result) {
		return nullptr;
	}
	
	Result* source = static_cast<Result*>(result);
 	const long long end = source->start + source->length;
 	long long sum = 0;
 	const int *array = source->array;
 	
	for(long long i = source->start; i < end; ++i)
	{
		sum += array[i];
	}
	
	source->result = sum;	
	return nullptr;
}
 
int main()
{
	srand((unsigned)time(NULL));  
	int* array = new int[N];
	
	rand(array);
	Result result[THREAD_NUM];
	pthread_t threads[THREAD_NUM];
	long long length = N/THREAD_NUM;
			clock_t start, finish;  
    double  duration;
    start = clock();
	for(int i=0;i<THREAD_NUM;i++)
	{
	 	result[i].array=array;
	 	result[i].start = i * length;
	 	result[i].length = length;
	 	result[i].result = 0;
	 	result[i].index = i;
		pthread_create(&threads[i],NULL,func,(void*)(&result[i]));	                                                
	}           
	
 
	for(long long i=0;i<THREAD_NUM;i++)    
	{
		long long ret = pthread_join(threads[i],NULL);
	}
	
 
	long long sum = 0;
	for (long long i = 0; i < THREAD_NUM; ++i) {
		sum += result[i].result;
	}
	
	finish = clock();  
	duration = (double)(finish - start) / CLOCKS_PER_SEC;  
	printf( "%f seconds\n", duration );
 
	delete [] array;
	printf("sum:%lld\n", sum);                     
}