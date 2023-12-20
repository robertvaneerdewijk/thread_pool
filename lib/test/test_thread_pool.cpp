//
// Created by aq on 20-12-23.
//

//
// Created by aq on 08-04-20.
//

#include <algorithm>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

#include <gtest/gtest.h>

#include <thread_pool.hpp>

static std::vector<int> GenerateRandomVector(size_t size)
{
    using value_type = int;
    // We use static in order to instantiate the random engine
    // and the distribution once only.
    // It may provoke some thread-safety issues.
    static std::uniform_int_distribution<value_type> distribution(
            std::numeric_limits<value_type>::min(),
            std::numeric_limits<value_type>::max());
    static std::default_random_engine generator;

    std::vector<value_type> data(size);
    std::generate(data.begin(), data.end(), []() { return distribution(generator); });
    return data;
}
constexpr size_t kNumThreads = 4;
constexpr size_t kNumTasks = 16;
constexpr size_t kTaskSize = 100000;

TEST(ThreadPool, SortMultipleVectors) {

    std::vector<std::vector<int>> vectors;
    for (size_t i = 0; i < kNumTasks; ++i) {
        vectors.push_back(GenerateRandomVector(kTaskSize));
    }

    ThreadPool thread_pool{kNumThreads};

    // Create tasks and push them to the thread pool.
    // Store all futures in a vector for later use.
    std::vector<std::future<std::vector<int>&>> futures;
    {
        for (auto & vec : vectors) {
            futures.push_back(thread_pool.push([&vec](size_t id) -> std::vector<int>& {
                std::this_thread::sleep_for(std::chrono::milliseconds{1});
                std::sort(vec.begin(), vec.end());
                return vec;
            }));
        }
    }

    {
        size_t count = 0;
        while (count < kNumTasks) {
            for (auto &f : futures) {

                // The std::future is only valid if we did not call get() or share().
                // Not allowed to call get() or share() on an invalid future
                if (f.valid()) {

                    // Wait for a short enough time to trigger timeout to showcase parallel execution.
                    auto const status = f.wait_for(std::chrono::microseconds{2000});
                    if (status != std::future_status::timeout) {
                        std::cout << "ready\n" << std::flush;
                        auto const &vec = f.get();
                        ASSERT_TRUE(std::is_sorted(vec.begin(), vec.end()));
                        ++count;
                    } else {
                        std::cout << "not ready\n" << std::flush;
                    }
                }
            }
        }
    }

    for (auto const& vec : vectors) {
        ASSERT_TRUE(std::is_sorted(vec.begin(), vec.end()));
    }
}
