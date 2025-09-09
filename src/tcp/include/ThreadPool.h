#pragma once
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

class ThreadPool {
 public:
  explicit ThreadPool(unsigned int _size = std::thread::hardware_concurrency());
  ThreadPool(const ThreadPool &other) = delete;
  ThreadPool(ThreadPool &&other) = delete;
  ThreadPool &operator=(const ThreadPool &other) = delete;
  ThreadPool &operator=(ThreadPool &&other) = delete;
  ~ThreadPool();

  template <class F, class... Args>
  auto Add(F &&f, Args &&... args) -> std::future<typename std::invoke_result_t<F, Args...>>;

 private:
  std::vector<std::thread> threads_;
  std::queue<std::function<void()>> tasks_;
  std::mutex tasks_mtx_;
  std::condition_variable cv_;
  std::atomic<bool> stop_{false};
};

template <class F, class... Args>
auto ThreadPool::Add(F &&f, Args &&... args) -> std::future<typename std::invoke_result_t<F, Args...>> {
  using return_type = typename std::invoke_result_t<F, Args...>;

  // 用packed_task包装函数f，并用bing直接填入参数，通过packaged_task.get_future()获取返回值
  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));  // forward<T>返回T&&通过引用折叠可以保持类型的信息

  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(tasks_mtx_);
    if (stop_) {
      throw std::runtime_error("ThreadPool already stop, can't add task");
    }
    tasks_.emplace([task]() { (*task)(); });
  }
  cv_.notify_one();  // 唤醒一个线程执行该任务
  return res;
}
