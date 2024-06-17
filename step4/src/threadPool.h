#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

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
    std::queue<std::function<void()>> tasks;

    // 互斥量和条件变量用于线程同步
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// 构造函数，启动指定数量的工作线程
inline ThreadPool::ThreadPool(size_t num_threads) : stop(false) {
    for (size_t i = 0; i < num_threads; ++i)
        workers.emplace_back([this] { worker_thread(); });
}

// 析构函数，等待所有线程完成后销毁线程池
inline ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
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
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // 不允许在关闭线程时添加更多的任务
        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        // 向队列中添加任务
        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}

// 工作线程函数，从队列中取出任务并执行
inline void ThreadPool::worker_thread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            // 等待直到队列非空或者线程池被销毁
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty())
                return;
            // 取出队列中的任务
            task = std::move(tasks.front());
            tasks.pop();
        }
        // 执行任务
        task();
    }
}

// 使用示例
void example_task(int num) {
    std::cout << "Task " << num << " is running\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Task " << num << " is done\n";
}
