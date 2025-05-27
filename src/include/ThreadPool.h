#pragma once
#include <functional>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

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
    // void add(std::function<void()>);  Add task to the queue
    template<class F, class... Args>
    auto add(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>; 
};


template<class F, class... Args>
auto ThreadPool::add(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(tasks_mtx);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){ (*task)(); });
    }
    cv.notify_one();
    return res;
}