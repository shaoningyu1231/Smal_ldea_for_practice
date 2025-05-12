#include <iostream>
#include <vector>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#ifndef THREADPOOL_H
#define THREADPOOL_H

// Task structure
struct Task {
	void (*task_func)(void* arg);
	void* user_data;

	// Task queue structure
	struct Task* prev;
	struct Task* next;

};
typedef struct Manager manager_t;

// Thread structure
typedef struct Worker {
	pthread_t thread;
	int id;
	int terminate;

	manager_t* pool;
	struct Worker* prev;
	struct Worker* next;
}Worker;

// Thread pool structure
struct Manager {

	Worker* workers;
	Task* tasks;

	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int shutdown;
	int num_threads;
};

static void* thread_callback(void* arg);
manager_t* ThreadPool_create(int num_threads);
int ThreadPool_add_task(manager_t* pool, void (*task_func)(void* arg), void* user_data);
int ThreadPool_destroy(manager_t* pool);



#endif // THREADPOOL_H