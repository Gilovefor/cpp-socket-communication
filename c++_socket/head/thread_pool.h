#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <utility>
#include <future>

class Thread_Pool {
private:
    std::queue<std::function<void()>> Task_queue;  // 任务队列
    std::vector<std::thread> Thread_container;    // 线程容器
    std::mutex mtx;          // 任务队列互斥锁
    std::mutex cout_mtx;     // 控制台输出互斥锁
    std::condition_variable condition;  // 条件变量
    bool stop;               // 线程池是否关闭

public:
    Thread_Pool(int numThreads);
    ~Thread_Pool();

    // 任务入队模板方法
    template <typename F, typename... Args>
    void enqueue(F&& f, Args&&... args);

    // 线程安全打印信息
    void thread_safe_print(int task_id, std::thread::id thread_id, const std::string& status);
};

// **模板方法定义**
template <typename F, typename... Args>
void Thread_Pool::enqueue(F&& f, Args&&... args) {
    auto task = std::make_shared<std::packaged_task<void()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    {
        std::unique_lock<std::mutex> lock(mtx);
        Task_queue.emplace([task]() { (*task)(); });
    }

    condition.notify_one();  // 唤醒一个线程执行任务
}

#endif
