## 使用boost的无锁队列

遇到的问题
1. boost::lockfree::queue<T> 的构造函数需要指定队列的大小，否则会报错。

2. boost::lockfree::queue<T> 的push和pop函数需要指定T的类型，否则会报错。

3. boost::lockfree::queue<T> 的T要满足平凡析构函数，否则会报错。

错误示例：
```
class ThreadPool {
  public:
    ...
    // 任务队列
    boost::lockfree::queue<std::function<void()>> tasks;
    std::atomic<bool> stop;
};

```

```
# 报错
(boost::has_trivial_assign<T>::value)
        BOOST_STATIC_ASSERT((boost::has_trivial_assign<T>::value));
```
错误表明正在使用的类型 T 需要有一个平凡析构函数（trivial destructor），而你提供的类型不满足这个要求。Boost 的无锁队列（lockfree queue）需要确保存储的类型可以在不调用析构函数的情况下进行销毁，以确保无锁操作的正确性和效率。

解决办法：

* 确保类型具有平凡析构函数： 确保你放入队列的类型是一个简单的类型，例如基础数据类型或具有平凡析构函数的结构体。

* 使用指针： 如果你的类型无法拥有平凡析构函数，可以考虑将类型包装在指针中，这样队列存储的是指针，而不是对象本身。

* 实现平凡析构函数： 如果你有自定义的类型，确保它的析构函数是平凡的。这意味着析构函数不做任何复杂的操作。

修复代码示例：
```
class ThreadPool {
  public:
    ...
    // 任务队列
    boost::lockfree::queue<std::function<void()>* > tasks;
    std::atomic<bool> stop;
    
};


auto ThreadPool::enqueue(F &&f,
                         Args &&...args) -> std::future<decltype(f(args...))> {
    using return_type = decltype(f(args...));
    ...
    // 向队列中添加任务// 向队列中添加任务
    auto wrapped_task = new std::function<void()>([task]() { (*task)(); });
    while (!tasks.push(wrapped_task)) {
    }
    return res;
}

inline void ThreadPool::worker_thread() {
    while (!stop.load()) {
        std::function<void()> *task; # 使用指针
        if (tasks.pop(task)) {
            (*task)(); # 调用任务
            delete task; # 释放任务
        } else {
            std::this_thread::yield(); // 防止 busy waiting
        }
    }
}
```

智能指针（如C++中的std::unique_ptr和std::shared_ptr）的析构函数并不总是平凡的（trivial），这取决于智能指针的实现方式及其管理的资源类型。

1. std::unique_ptr：

   * std::unique_ptr的析构函数通常是非平凡的，因为它需要在销毁时释放它所管理的资源。这意味着它的析构函数会调用托管对象的析构函数，然后释放分配的内存。
   * 具体来说，当std::unique_ptr管理动态分配的对象时，其析构函数是非平凡的。
2. std::shared_ptr：

   * std::shared_ptr的析构函数通常也是非平凡的，因为它需要管理引用计数。析构函数会减少引用计数，当引用计数变为零时，它会释放托管的资源。
   * 因此，std::shared_ptr的析构函数也是非平凡的。
对于这两种智能指针，它们的析构函数都涉及额外的操作（如释放资源或管理引用计数），因此通常被认为是非平凡的。

## 平凡析构函数（Trivial Destructor）的定义
一个类的析构函数在以下条件下被认为是平凡的：

* 它在用户代码中没有显式定义。
* 它不调用基类的析构函数（除非是隐式调用）。
* 它不调用类成员的析构函数（除非是隐式调用）。
* 它不是虚函数。
* 编译器认为它不需要执行任何特殊操作。
  
由于智能指针的析构函数需要管理资源的释放，符合上述条件的可能性较小，因此通常不被认为是平凡的。总结来说，智能指针的析构函数一般是非平凡的，因为它们需要执行特定的资源管理操作。