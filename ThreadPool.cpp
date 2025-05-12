#include "ThreadPool.h"

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

// Function to create a new thread pool
manager_t* ThreadPool_create(int num_threads) {
	// Check the allocation
	if (num_threads <= 0) {
		fprintf(stderr, "Invalid number of threads\n");
		return NULL;
	}
	manager_t* pool = (manager_t*)malloc(sizeof(manager_t));
	if (pool == NULL) {
		perror("Failed to allocate memory for thread pool\n");
		return NULL;
	}
	
	memset(pool, 0, sizeof(manager_t));
	pool->num_threads = num_threads;
	pool->shutdown = 0;
	
	// Initialize mutex and condition variable
	if (pthread_mutex_init(&pool->mutex, NULL) != 0) {
		perror("Failed to initialize mutex\n");
		free(pool);
		exit(1);
	}
	if (pthread_cond_init(&pool->cond, NULL) != 0) {
		perror("Failed to initialize condition variable\n");
		pthread_mutex_destroy(&pool->mutex);
		free(pool);
		exit(1);
	}

	// Create worker threads
	for (int idx = 0; idx < num_threads; idx++) {
		Worker* worker = (Worker*)malloc(sizeof(Worker));
		if (worker == NULL) {
			perror("Failed to allocate memory for worker thread\n");
			ThreadPool_destroy(pool);
			exit(1);
		}
		memset(worker, 0, sizeof(Worker));
		worker->pool = pool;
		worker->id = idx;
		worker->terminate = 0;

		// Create the thread
		int ret = pthread_create(&worker->thread, NULL, thread_callback, worker);
		if (ret != 0) {
			perror("Failed to create thread\n");
			free(worker);
			ThreadPool_destroy(pool);
			exit(1);
		}

		LINKED_LIST_INSERT(worker, pool->workers);
	}
	printf("Thread pool created with %d threads\n", num_threads);
	return pool;
}

// Entry point for worker threads
static void* thread_callback(void* arg) {
	Worker* worker = (Worker*)arg;
	manager_t* pool = worker->pool;
	printf("Thread %d started\n", worker->id);

	while (1) {
		pthread_mutex_lock(&pool->mutex);
		while (pool->tasks == NULL && !pool->shutdown) {
			pthread_cond_wait(&pool->cond, &pool->mutex);
		}
		if (pool->shutdown) {
			pthread_mutex_unlock(&pool->mutex);
			printf("Worker %d terminating due to shutdown\n", worker->id);
			pthread_exit(NULL);
		}
		
		Task* task = pool->tasks;
		if (task != NULL) {
			LINKED_LIST_REMOVE(task, worker->pool->tasks);
			
		}
		pthread_mutex_unlock(&pool->mutex);
		if (task != NULL) {
			printf("Worker %d executing task\n", worker->id);
			task->task_func(task->user_data);
			free(task);
		}
		
	}
	return NULL;
}

// Function to add a task to the thread pool
int ThreadPool_add_task(manager_t* pool, void (*task_func)(void* arg), void* user_data) {
	if (pool == NULL || task_func == NULL) {
		fprintf(stderr, "Invalid arguments to add task\n");
		return -1;
	}
	Task* task = (Task*)malloc(sizeof(Task));
	if (task == NULL) {
		perror("Failed to allocate memory for task\n");
		exit(1);
	}
	task->task_func = task_func;
	task->user_data = user_data;
	task->prev = NULL;
	task->next = NULL;

	pthread_mutex_lock(&pool->mutex);

	// Do not add task if pool is shutting down
	if (pool->shutdown) {
		pthread_mutex_unlock(&pool->mutex);
		free(task);
		fprintf(stderr, "Failed to add task to a shutting down pool\n");
		return -1;
	}
	LINKED_LIST_INSERT(task, pool->tasks);
	pthread_cond_signal(&pool->cond);
	pthread_mutex_unlock(&pool->mutex);
	return 0;
}

// Function to destroy the thread pool
int ThreadPool_destroy(manager_t* pool) {
	if (pool == NULL) {
		return -1;
	}
	printf("Initiating shutdown of thread pool\n");
	pthread_mutex_lock(&pool->mutex);

	// Check if pool is already shutting down
	if (pool->shutdown) {
		pthread_mutex_unlock(&pool->mutex);
		fprintf(stderr, "Thread pool is already shutting down\n");
		return -1;
	}
	pool->shutdown = 1;
	
	// Wake up all threads
	pthread_cond_broadcast(&pool->cond);
	pthread_mutex_unlock(&pool->mutex);

	// Wait for all threads to finish
	Worker* cur_worker = pool->workers;
	std::vector<pthread_t> threads_ids;
	while (cur_worker != NULL) {
		pthread_join(cur_worker->thread, NULL);
		cur_worker = cur_worker->next;
	}
	printf("Joining %zu worker threads...\n", threads_ids.size());
	for (pthread_t thread_id : threads_ids) {
		if (pthread_join(thread_id, NULL) != 0) {
			perror("Failed to join thread\n");
		};
	}
	printf("All threads joined successfully\n");

	// Free remaining tasks
	pthread_mutex_lock(&pool->mutex);
	Task* cur_task = pool->tasks;
	while (cur_task != NULL) {
		Task* next_task = cur_task->next;
		free(cur_task);
		cur_task = next_task;
	}
	pool->tasks = NULL;
	pthread_mutex_unlock(&pool->mutex);

	// Free worker structure
	cur_worker = pool->workers;
	while (cur_worker != NULL) {
		Worker* next_worker = cur_worker->next;
		free(cur_worker);
		cur_worker = next_worker;
	}
	pool->workers = NULL;

	// Free mutex and condition variable
	pthread_mutex_destroy(&pool->mutex);
	pthread_cond_destroy(&pool->cond);
	free(pool);
	printf("Thread pool destroyed successfully\n");
	return 0;
}

// Example task function for testing
void example_task(void* arg) {
	if (arg == NULL) {
		fprintf(stderr, "Invalid argument to task function\n");
		return;
	}
	int id = *(int*)arg;
	std::cout << "Task executed by thread " << id << std::endl;
	// sleep(1);
	free(arg);
}


int main() {
	int num_tasks = 20;
	int nthreads = 4;

	// Create thread pool
	manager_t* pool = ThreadPool_create(nthreads);
	if (pool == NULL) {
		std::cerr << "Failed to create thread pool" << std::endl;
		return -1;
	}
	
	// Add tasks to the thread pool
	for (int i = 0; i < num_tasks; i++) {
		int* arg = (int*)malloc(sizeof(int));
		if (arg == NULL) {
			perror("Failed to allocate memory for task argument\n");
			continue;
		}
		*arg = i;
		if (ThreadPool_add_task(pool, example_task, arg) != 0) {
			std::cerr << "Failed to add task to thread pool" << std::endl;
			free(arg);
		}
	}
	if (ThreadPool_destroy(pool) != 0) {
		std::cerr << "Failed to destroy thread pool" << std::endl;
		return -1;
	}
	printf("Thread pool destroyed successfully\n");
	return 0;
}
