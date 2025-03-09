#include "../head/thread_pool.h"

// **构造函数**
Thread_Pool::Thread_Pool(int numThreads) : stop(false) {
    for (int i = 0; i < numThreads; i++) {
        Thread_container.emplace_back([this] {
            while (true) {
                std::function<void()> func;
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    condition.wait(lock, [this] { return stop || !Task_queue.empty(); });

                    if (stop && Task_queue.empty())
                        return;

                    func = std::move(Task_queue.front());
                    Task_queue.pop();
                }
                func();
            }
            });
    }
}

// **析构函数**
Thread_Pool::~Thread_Pool() {
    {
        std::unique_lock<std::mutex> lock(mtx);
        stop = true;
    }
    condition.notify_all();

    for (std::thread& thread : Thread_container) {
        thread.join();
    }
}

// **线程安全打印**
void Thread_Pool::thread_safe_print(int task_id, std::thread::id thread_id, const std::string& status) {
    std::lock_guard<std::mutex> lock(cout_mtx);
    std::cout << "任务 " << task_id << " 正在 " << status << "，线程 ID: " << thread_id << std::endl;
}
