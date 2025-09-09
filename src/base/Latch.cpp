#include "Latch.h"
#include <thread>
#include <condition_variable>

Latch::Latch(int count) : count_(count) {}

Latch::~Latch() {}

void Latch::wait() {
  std::unique_lock<std::mutex> lock(mtx_);
  while(count_ > 0) {
    cv_.wait(lock);
  }
}

void Latch::notify() {
  std::unique_lock<std::mutex> lock(mtx_);
  --count_;
  if(count_ == 0) {
    cv_.notify_all();
  }
}