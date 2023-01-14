#include <pthread.h>
#include <stdio.h>
#ifndef TS_QUEUE_HPP
#define TS_QUEUE_HPP

#define DEFAULT_BUFFER_SIZE 200

template <class T>
class TSQueue {
public:
	// constructor
	TSQueue();

	explicit TSQueue(int max_buffer_size);

	// destructor
	~TSQueue();

	// add an element to the end of the queue
	void enqueue(T item);

	// remove and return the first element of the queue
	T dequeue();

	// return the number of elements in the queue
	int get_size();

	bool is_empty();

	bool is_full();
private:
	// the maximum buffer size
	int buffer_size;
	// the buffer containing values of the queue
	T* buffer;
	// the current size of the buffer
	int size;
	// the index of first item in the queue
	int head;
	// the index of last item in the queue
	int tail;
	

	// pthread mutex lock
	pthread_mutex_t mutex;
	// pthread conditional variable
	pthread_cond_t cond_enqueue, cond_dequeue;
};

// Implementation start

template <class T>
TSQueue<T>::TSQueue() : TSQueue(DEFAULT_BUFFER_SIZE) {
}

template <class T>
TSQueue<T>::TSQueue(int buffer_size) : buffer_size(buffer_size),head(0), tail(0), size(0) {
	pthread_mutex_init(&mutex, 0);
	pthread_cond_init(&cond_enqueue,NULL);
	pthread_cond_init(&cond_dequeue,NULL);
	buffer = new T[buffer_size];                                 //allocate the space for queue
}

template <class T>
TSQueue<T>::~TSQueue() {
	pthread_mutex_destroy(&mutex);
	delete buffer;                                               //deallocate the space of queue
}

template <class T>
void TSQueue<T>::enqueue(T item) {
	pthread_mutex_lock(&mutex);
	while(is_full())
	{
		printf("queue is full. waiting.\n");
		pthread_cond_wait(&cond_enqueue,&mutex);
	}
	//printf("queue \n");
	buffer[ tail%buffer_size ] = item;
	++tail;
	++size;
	pthread_cond_signal(&cond_dequeue);
	pthread_mutex_unlock(&mutex);
}

template <class T>
T TSQueue<T>::dequeue() {
	
	T item;
	pthread_mutex_lock(&mutex);
	while(is_empty())
	{
		printf("queue is empty. waiting.\n");
		pthread_cond_wait(&cond_dequeue,&mutex);
	}
	item = buffer[head%buffer_size];
	//printf("dequeue \n");
	++head;
	--size;
	pthread_cond_signal(&cond_enqueue);
	pthread_mutex_unlock(&mutex);
	return item;
}

template <class T>
int TSQueue<T>::get_size() {
	int i;
	pthread_mutex_lock(&mutex);
	i=size;
	pthread_mutex_unlock(&mutex);
	return size;
}

template <class T> 
bool TSQueue<T>::is_empty()
{
	if(size==0)
	{
		return true;
	}
	return false;
}

template <class T>
bool TSQueue<T>::is_full()
{
	if(size==buffer_size)
	{
		return true;
	}
	
	return false;
}
#endif // TS_QUEUE_HPP
