#include "AsyncLogging.h"
#include "LogFile.h"

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <functional>

AsyncLogging::AsyncLogging() : running_(false), filepath_(nullptr), latch_(1) {
  current_ = std::make_unique<Buffer>();
  next_ = std::make_unique<Buffer>();
}

AsyncLogging::AsyncLogging(const char* filepath) : running_(false), filepath_(filepath), latch_(1) {
  current_ = std::make_unique<Buffer>();
  next_ = std::make_unique<Buffer>();
}

AsyncLogging::~AsyncLogging() {
  if(running_) {
    Stop();
  }
}

void AsyncLogging::Start() {
  running_ = true;
  thread_ = std::thread(std::bind(&AsyncLogging::ThreadFunc, this));

  // 等待线程启动
  latch_.wait();
}

void AsyncLogging::Stop() {
  running_ = false;
  // 唤醒后端线程
  cv_.notify_all();
  // 等待线程结束
  if(thread_.joinable()) {
    thread_.join();
  }
}

void AsyncLogging::Flush() {
  fflush(stdout);
}

void AsyncLogging::Append(const char* data, int len) {
  std::unique_lock<std::mutex> lock(mutex_);  // 上锁

  if(current_->Available() >= len) {
    current_->Append(data, len);
  }
  else {  // current_空间不够
    buffers_.push_back(std::move(current_));  // 将current_加入已满缓冲区

    if(next_) {  // 如果next_存在，即next_还没有被使用
      current_ = std::move(next_);
    }
    else {  // next_已经被使用了，则申请新的内存
      current_.reset(new Buffer());
    }

    // 写入数据
    current_->Append(data, len);
  }

  // 唤醒后端线程
  cv_.notify_one();
}

void AsyncLogging::ThreadFunc() {
  std::unique_ptr<Buffer> new_current = std::make_unique<Buffer>();
  std::unique_ptr<Buffer> new_next = std::make_unique<Buffer>();

  // std::unique_ptr<LogFile> logfile = std::make_unique<LogFile>();
  std::unique_ptr<LogFile> logfile = std::make_unique<LogFile>("../mylog/log1.log");

  new_current->Bzero();
  new_next->Bzero();

  std::vector<std::unique_ptr<Buffer>> active_buffers;

  // 创建成功，提醒主线程
  latch_.notify();

  while(running_) {
    std::unique_lock<std::mutex> lock(mutex_);  // 上锁

    if(buffers_.empty()) {
      // 还没有缓冲区满，等待一段时间
      cv_.wait_until(lock, std::chrono::system_clock::now() + BufferWriteTimeout * std::chrono::milliseconds(1000),
                    []{ return false; });
    }

    // 将当前缓冲区加入到已满缓冲区
    buffers_.push_back(std::move(current_));
    // 将已满缓冲区与即将写入的缓冲区交换
    active_buffers.swap(buffers_);

    // 更换为新的缓冲区
    current_ = std::move(new_current);

    if(!next_) {  // 备用缓冲区已经使用，则跟换为新的缓冲区
      next_ = std::move(new_next);
    }

    // 写入日志文件
    for (const auto& buffer : active_buffers) {
      logfile->Write(buffer->Data(), buffer->Size());
    }

    // 写入的数据达到限制的最大
    if(logfile->GetWrittenBytes() >= FileMaximumSize) {
      // 指向一个新的日志文件
      logfile.reset(new LogFile(filepath_));
    }

    if(active_buffers.size() > 2){
      // 留两个缓冲区，用于后续
      active_buffers.resize(2);
    }

    if(!new_current) {  // 新的当前缓冲区已经使用
      if(!active_buffers.empty()) {
        new_current = std::move(active_buffers.back());
        active_buffers.pop_back();
      }
      else {
        new_current.reset(new Buffer());
      }
      new_current->Bzero();  // 初始化
    }

    if(!new_next) {  // 新的备用缓冲区已经使用
      if(!active_buffers.empty()) {
        new_next = std::move(active_buffers.back());
        active_buffers.pop_back();
      }
      else {
        new_next.reset(new Buffer());
      }
      new_next->Bzero();  // 初始化
    }

    active_buffers.clear();
  }
}
