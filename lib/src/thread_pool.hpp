#ifndef LIB_THREAD_POOL_HPP_
#define LIB_THREAD_POOL_HPP_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

#include "queue.hpp"

class ThreadPool {
public:
    explicit ThreadPool(size_t nthreads)
            : threads_{}
            , stop_{false}
    {
        for (size_t id = 0; id < nthreads; ++id) {
            threads_.emplace_back(std::thread(&ThreadPool::thread, this, id));
        }
    }

    ~ThreadPool() {
        stop();
    }

    void stop() {
        stop_ = true;
        size_t num_threads_joined = 0;
        while(num_threads_joined < threads_.size()) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                cv_.notify_all();
            }
            for (std::thread &thread : threads_) {
                if (thread.joinable()) {
                    thread.join();
                    ++num_threads_joined;
                }
            }
        }
        queue_.clear();
    }

    template<typename Functor>
    auto push(Functor && functor) -> std::future<decltype(functor(0))> {
        auto pack = std::make_shared<std::packaged_task<decltype(functor(0))(size_t)>>(std::forward<Functor>(functor));
        queue_.push([pack](size_t id){ (*pack)(id); });
        std::lock_guard<std::mutex> lock(mutex_);
        cv_.notify_one();
        return pack->get_future();
    }

    template<typename Functor, typename... Args>
    auto push(Functor && functor, Args&&... args) -> std::future<decltype(functor(0, args...))> {
        return Push(std::bind(std::forward<Functor>(functor), std::placeholders::_1, std::forward<Args>(args)...));
    }

private:
    std::vector<std::thread> threads_;
    std::atomic<bool> stop_;
    Queue<std::function<void(size_t)>> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;

    void thread(size_t const id) {
        while (!stop_) {
            while (!queue_.is_empty()) {
                try {
                    std::function<void(size_t)> work;
                    if (queue_.pop(&work)) {
                        work(id);
                    }
                } catch (std::exception const& e) {
                    std::flush(std::cerr);
                    std::cerr << "\n\nCaught exception inside thread_pool.hpp functor:\n" << e.what();
                    std::flush(std::cerr);
                    return;
                }
            }
            // queue is empty, wait for work
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [&]() { return !queue_.is_empty() || stop_; });
        }
    }
};

#endif //LIB_THREAD_POOL_HPP_
