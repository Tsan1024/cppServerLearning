
#pragma once

#include <boost/lockfree/queue.hpp>
#include <functional>
#include <future>
#include <vector>

namespace ThreadPoolErrorExample {

class ThreadPool {
  public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    // 添加任务到线程池
    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args) -> std::future<decltype(f(args...))>;

  private:
    // 线程需要执行的工作函数
    void worker_thread();

    // 线程池中的工作线程
    std::vector<std::thread> workers;

    // 任务队列
    boost::lockfree::queue<std::function<void()>> tasks;

    std::atomic<bool> stop;
};

// 构造函数，启动指定数量的工作线程
inline ThreadPool::ThreadPool(size_t num_threads) : stop(false) {
    for (size_t i = 0; i < num_threads; ++i)
        workers.emplace_back([this] { worker_thread(); });
}

// 析构函数，等待所有线程完成后销毁线程池
inline ThreadPool::~ThreadPool() {
    stop.store(true);
    for (std::thread &worker : workers)
        worker.join();
}

// 添加任务到线程池的队列中
template <class F, class... Args>
auto ThreadPool::enqueue(F &&f,
                         Args &&...args) -> std::future<decltype(f(args...))> {
    using return_type = decltype(f(args...));
    // 创建一个任务指向的智能指针，使它可以异步地获取值或异常
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();

    // 向队列中添加任务
    while (!tasks.push([task]() { (*task)(); })) {
    }

    return res;
}

// 工作线程函数，从队列中取出任务并执行
inline void ThreadPool::worker_thread() {
    while (!stop.load()) {
        std::function<void()> task;
        if (tasks.pop(task)) {
            task();
        } else {
            std::this_thread::yield(); // 防止 busy waiting
        }
    }
}

} // namespace ThreadPoolErrorExample
