#include "ThreadPool.h"

ThreadPool::ThreadPool(unsigned int _size) {
  for (unsigned int i = 0; i != _size; ++i) {
    threads_.emplace_back(std::thread([this]() {
      while (true) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(tasks_mtx_);
          cv_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
          if (stop_ && tasks_.empty()) {  // 线程池停止，并且所有任务都执行完
            return;
          }
          task = tasks_.front();
          tasks_.pop();
        }
        task();
      }
    }));
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(tasks_mtx_);
    stop_ = true;  // 关闭线程池
  }
  cv_.notify_all();                   // 通知所有线程
  for (std::thread &th : threads_) {  // 等待所有线程执行完后关闭线程回收资源
    if (th.joinable()) {
      th.join();
    }
  }
}
