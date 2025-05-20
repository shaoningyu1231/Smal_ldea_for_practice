#pragma once
#include <functional>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

class ThreadPool
{
private:
    /* data */
    std::vector<std::thread> threads; // Thread pool
    std::queue<std::function<void()>> tasks; // Task queue
    std::mutex tasks_mtx; // Mutex for task queue
    std::condition_variable cv; // Condition variable for task queue
    bool stop; // Flag to indicate if the thread pool is stopped
public:
    ThreadPool(/* size = */ int size = 10); // Constructor
    ~ThreadPool();
    void add(std::function<void()>); // Add task to the queue
};
