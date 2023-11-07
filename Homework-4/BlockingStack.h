

#ifndef BlockingStack_h
#define BlockingStack_h

#include <stack>
#include <mutex>
#include <memory>
#include <exception>

namespace threadPerf {

    struct empty_stack : public std::exception {
        const char* what() const noexcept override { return nullptr; }
    };

    template <typename T>
    class BlockingStack {

    private:
        std::stack<T> st;
        mutable std::mutex mtx;


        BlockingStack(const BlockingStack& other, const std::lock_guard<std::mutex>&) : st{ other.st }, mtx{} {}

    public:

        BlockingStack() = default;
        BlockingStack(const BlockingStack& other) : BlockingStack{ other, std::lock_guard {other.mtx} } {}
        BlockingStack(BlockingStack&& other) = delete;
        BlockingStack& operator=(BlockingStack&& rhs) = delete;
        BlockingStack& operator=(const BlockingStack& other) {
            if (this == &other) return *this;
            BlockingStack tmp{ other };
            st = std::move(tmp.st);
            return *this;
        }

        ~BlockingStack() = default;

        void push(T&& val) {
            std::lock_guard lc{ mtx };
            st.push(std::forward<T>(val));
        }

        std::shared_ptr<T> pop() {
            std::lock_guard lc{ mtx };
            if (st.empty()) return nullptr;
            std::shared_ptr<T> res = std::make_shared<T>(std::move(st.top()));
            st.pop();
            return res;
        }

        void pop(T& place) {
            std::lock_guard lc{ mtx };
            if (st.empty()) return;
            place = std::move(st.top());
            st.pop();
        }

        bool empty() {
            std::lock_guard lc{ mtx };
            return st.empty();
        }
    };

}
#endif
