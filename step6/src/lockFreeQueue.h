#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

template <typename T> class LockFreeQueue {
  public:
    LockFreeQueue() {
        Node *node = new Node();
        head.store(node);
        tail.store(node);
    }

    ~LockFreeQueue() {
        while (Node *node = head.load()) {
            head.store(node->next);
            delete node;
        }
    }

    void enqueue(T value) {
        Node *node = new Node(std::move(value));
        Node *prevTail = tail.exchange(node);
        prevTail->next.store(node, std::memory_order_release);
    }

    bool dequeue(T &result) {
        int spin_count = 1000; // 设定最大自旋次数
        Node *node;
        do {
            node = head.load();
            Node *next = node->next.load(std::memory_order_acquire);
            if (next == nullptr) {
                // 自旋超过最大次数后休眠一段时间
                if (--spin_count <= 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    spin_count = 1000; // 重置自旋计数
                }
                continue;
            }
            if (head.compare_exchange_weak(node, next)) {
                result = std::move(next->value);
                delete node; // 应该删除 next 节点而不是 node
                return true;
            }
        } while (true);
    }

  private:
    struct Node {
        T value;
        std::atomic<Node *> next;

        Node() : next(nullptr) {}
        Node(T val) : value(std::move(val)), next(nullptr) {}
    };

    std::atomic<Node *> head;
    std::atomic<Node *> tail;
};
