#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <stdio.h>
#include "consumer.hpp"
#include "ts_queue.hpp"
#include "item.hpp"
#include "transformer.hpp"


#ifndef CONSUMER_CONTROLLER
#define CONSUMER_CONTROLLER

class ConsumerController : public Thread {
public:
	// constructor
	ConsumerController(
		TSQueue<Item*>* worker_queue,
		TSQueue<Item*>* writer_queue,
		Transformer* transformer,
		int check_period,
		int low_threshold,
		int high_threshold
	);

	// destructor
	~ConsumerController();

	virtual void start();

private:
	std::vector<Consumer*> consumers_vector;

	TSQueue<Item*>* worker_queue;
	TSQueue<Item*>* writer_queue;

	Transformer* transformer;

	// Check to scale down or scale up every check period in microseconds.
	int check_period;
	// When the number of items in the worker queue is lower than low_threshold,
	// the number of consumers scaled down by 1.
	int low_threshold;
	// When the number of items in the worker queue is higher than high_threshold,
	// the number of consumers scaled up by 1.
	int high_threshold;

	static void* process(void* arg);
};

// Implementation start

ConsumerController::ConsumerController(
	TSQueue<Item*>* worker_queue,
	TSQueue<Item*>* writer_queue,
	Transformer* transformer,
	int check_period,
	int low_threshold,
	int high_threshold
) : worker_queue(worker_queue),
	writer_queue(writer_queue),
	transformer(transformer),
	check_period(check_period),
	low_threshold(low_threshold),
	high_threshold(high_threshold) {
}

ConsumerController::~ConsumerController() {}

void ConsumerController::start() {
	pthread_create(&t,0,ConsumerController::process,NULL);
}

void* ConsumerController::process(void* arg) {
	ConsumerController *con_con = (ConsumerController*) arg;
	Consumer *consumer;
	while(1)
	{
		double percentage;
		percentage = con_con->worker_queue->get_size()/4000;
		

		if(percentage>=con_con->high_threshold)
		{
			consumer = new Consumer(con_con->worker_queue,con_con->writer_queue,con_con->transformer);
			printf("Scaling up consumers from %d",con_con->consumers_vector.size());
			con_con->consumers_vector.push_back(consumer);
			printf("\b to %d",con_con->consumers_vector.size());
			con_con->consumers_vector.back()->start();
		}
		else if(percentage<con_con->low_threshold)
		{
			printf("Scaling down consumers from %d",con_con->consumers_vector.size());
			consumer = con_con->consumers_vector.back();
			con_con->consumers_vector.pop_back();
			printf("\b to %d",con_con->consumers_vector.size());
			consumer->cancel();
			delete consumer;
		}
		sleep(con_con->check_period);
	}
	
	return nullptr; 
}

#endif // CONSUMER_CONTROLLER_HPP
