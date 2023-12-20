#ifndef LIB_QUEUE_HPP_
#define LIB_QUEUE_HPP_

#include <mutex>
#include <queue>

template<typename T>
class Queue {
public:
    bool push(T const& x) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(x);
        return true;
    }
    bool push(T&& x) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(x));
        return true;
    }
    bool pop(T * x) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        } else {
            *x = queue_.front();
            queue_.pop();
            return true;
        }
    }
    [[nodiscard]] bool is_empty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        while(!queue_.empty()) {
            queue_.pop();
        }
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
};

#endif //LIB_QUEUE_HPP_
