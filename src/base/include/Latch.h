#pragma once
#include <thread>
#include <condition_variable>

class Latch {
 public:
  explicit Latch(int count);
  Latch(const Latch&) = delete;
  Latch& operator=(const Latch&) = delete;
  Latch(Latch&&) = delete;
  Latch& operator=(Latch&&) = delete;
  ~Latch();

  // 等待count为0
  void wait();

  // 令count-1，当count==0时唤醒所有等待的线程
  void notify();

 private:
  int count_;
  std::mutex mtx_;
  std::condition_variable cv_;
};