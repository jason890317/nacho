#include <pthread.h>
#include "thread.hpp"
#include "ts_queue.hpp"
#include "item.hpp"
#include "transformer.hpp"

#ifndef PRODUCER_HPP
#define PRODUCER_HPP

class Producer : public Thread {
public:
	// constructor
	Producer(TSQueue<Item*>* input_queue, TSQueue<Item*>* worker_queue, Transformer* transfomrer);

	// destructor
	~Producer();

	virtual void start();
private:
	TSQueue<Item*>* input_queue;
	TSQueue<Item*>* worker_queue;

	Transformer* transformer;

	// the method for pthread to create a producer thread
	static void* process(void* arg);
};

Producer::Producer(TSQueue<Item*>* input_queue, TSQueue<Item*>* worker_queue, Transformer* transformer)
	: input_queue(input_queue), worker_queue(worker_queue), transformer(transformer) {
}

Producer::~Producer() {}

void Producer::start() {
	pthread_create(&t,0,Producer::process,(void*)this);
}

void* Producer::process(void* arg) {
	Item* it;
	Producer* producer = (Producer*) arg;
	int new_val;
	while(1)
	{
		it=producer->input_queue->dequeue();
		printf("producer dequeue q1\n");
		new_val=producer->transformer->producer_transform(it->opcode,it->val);
	
		it->val=new_val;
		producer->worker_queue->enqueue(it);
		printf("producer enqueue q2\n");
    }
	return nullptr;	
}

#endif // PRODUCER_HPP
