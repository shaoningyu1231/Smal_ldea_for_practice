#include "ThreadPool.h"

ThreadPool::ThreadPool(int size) : stop(false) {
    for (int i = 0; i < size; ++i) {
        threads.emplace_back(std::thread([this]() {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(tasks_mtx); // Lock the mutex
                    // Wait for a task to be available or for the pool to stop
                    cv.wait(lock, [this]() {
                        return stop || !tasks.empty(); // Wait until there are tasks or the pool is stopping
                    });
                    if (stop && tasks.empty()) return;
                    
                    // Get the next task from the queue
                    task = tasks.front();
                    tasks.pop();
                }
                task();
            }
        }));
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(tasks_mtx); // Lock the mutex
        stop = true; // Set the stop flag to true
    }
    cv.notify_all(); // Notify all threads to wake up
    for (std::thread &th : threads) {
        if (th.joinable())
            th.join(); // Wait for all threads to finish
    }
}

void ThreadPool::add(std::function<void()> func) {
    {
        std::unique_lock<std::mutex> lock(tasks_mtx); // Lock the mutex
        if (stop)
            throw std::runtime_error("ThreadPool already stopped, can't add task any more");
        tasks.emplace(func); // Add the task to the queue
    }
    cv.notify_one(); // Notify one thread that a new task is available
}