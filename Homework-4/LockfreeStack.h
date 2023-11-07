

#ifndef LockfreeStack_h
#define LockfreeStack_h

#include <atomic>
#include <memory>

namespace threadPerf {

    template<typename T>
    class LockfreeStack {
    private:
        struct Node;
        struct counted_node_ptr {
            int external_count;
            Node* ptr;
        };

        struct Node {
            std::shared_ptr<T> data;
            std::atomic<int> internal_count;
            counted_node_ptr next;
            Node(T const& data_) : data{ std::make_shared<T>(data_) }, internal_count{ 0 } {}
        };

        std::atomic<counted_node_ptr> head;

        void increase_head_count(counted_node_ptr& old_counter) {
            counted_node_ptr new_counter;
            do
            {
                new_counter = old_counter;
                ++new_counter.external_count;
            } while (!head.compare_exchange_strong(old_counter, new_counter,
                std::memory_order_acquire,
                std::memory_order_relaxed));
            old_counter.external_count = new_counter.external_count;
        }

    public:
        LockfreeStack() = default;

        ~LockfreeStack() {
            while (pop());
        }
        bool empty() {
            auto val = pop();
            if (!val) return true;
            push(*val);
            return false;
        }

        void push(T const& data) {
            counted_node_ptr new_node;
            new_node.ptr = new Node(data);
            new_node.external_count = 1;
            new_node.ptr->next = head.load(std::memory_order_relaxed);
            while (!head.compare_exchange_weak(new_node.ptr->next, new_node,
                std::memory_order_release,
                std::memory_order_relaxed));
        }

        std::shared_ptr<T> pop() {
            counted_node_ptr old_head = head.load(std::memory_order_relaxed);
            for (;;) {
                increase_head_count(old_head);
                Node* const ptr = old_head.ptr;
                if (!ptr) {
                    return std::shared_ptr<T>();
                }

                if (head.compare_exchange_strong(old_head, ptr->next, std::memory_order_relaxed)) {
                    std::shared_ptr<T> res;
                    res.swap(ptr->data);
                    int const count_increase = old_head.external_count - 2;
                    if (ptr->internal_count.fetch_add(count_increase, std::memory_order_release) == -count_increase) {
                        delete ptr;
                    }
                    return res;
                }
                else if (ptr->internal_count.fetch_add(-1, std::memory_order_relaxed) == 1) {
                    ptr->internal_count.load(std::memory_order_acquire);
                    delete ptr;
                }
            }
        }
    };

}

#endif
