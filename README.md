# Thread Pool
Simple implementation of a thread pool using std::future, atomic bool, mutex, condition variable and a thread safe queue (another mutex in this case).

## Usage
```C++
#include <algorithm>
#include <ctime>
#include <vector>

#include <thread_pool.hpp>

int main(int argc, char* argv[]) {
  // creates thread pool with 4 threads
  ThreadPool thread_pool{4};
  
  // minimal example
  // task is wrapped in a lambda
  // the lambda shall always take task_id as its first argument
  auto future1 = thread_pool.push(
    [](size_t task_id) {
      // do some high latency work or take a nap
      std::this_thread::sleep_for(std::chrono::milliseconds{1000});
    }
  );
  
  // generate a vector of random ints
  std::srand(unsigned(std::time(nullptr)));
  std::vector<int> random_vector(1000);
  std::generate(random_vector.begin(), random_vector.end(), std::rand);

  // pass random vector as argument to the thread pool
  // first argument is still task_id
  auto future2 = thread_pool.push(
    [](size_t task_id, std::vector<int>& random_vector) {
      std::sort(random_vector.begin(), random_vector.end());
    },
    random_vector
  );
  
  
  // wait for result and get its value
  future2.wait();
  // random_vector is now sorted
  assert(std::is_sorted(random_vector.begin(), random_vector.end()));
  
  // wait for high latency work
  future1.wait();

}
```
