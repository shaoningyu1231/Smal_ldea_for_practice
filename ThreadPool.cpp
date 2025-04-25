#include <iostream>
#include <pthread.h>
#include <stdlib.h>

// Linked list macros
#define LINKED_LIST_INSERT(item, list) do { \
	(item)->prev = NULL; \
	(item)->next = (list); \
	if ((list) != NULL) { \
		(list)->prev = (item); \
	} \
	(list) = (item); \
} while(0)

#define LINKED_LIST_REMOVE(item, list) do { \
	if ((item)->prev != NULL) { \
		(item)->prev->next = (item)->next; \
	} else { \
		(list) = (item)->next; \
	} \
	if ((item)->next != NULL) { \
		(item)->next->prev = (item)->prev; \
	} \
} while(0)

// Task structure
struct Task {
	void (task_func)(void *arg);
	void *user_data;
	
	// Task queue structure
	struct name* prev;
	struct name* next;

};

// Thread structure
struct Worker {
	pthread_t thread;
	int id;
	int busy;

	struct Worker* prev;
	struct Worker* next;
};

// Thread pool structure
struct ThreadPool {
	
	struct Worker* workers;
	struct Task* tasks;
	
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};