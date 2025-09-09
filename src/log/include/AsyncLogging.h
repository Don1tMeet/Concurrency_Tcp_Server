#pragma once

#include "Latch.h"
#include "Logging.h"
#include <vector>
#include <memory>
#include <thread>

static const double BufferWriteTimeout = 3.0;  // 等待写入的时间
static const int64_t FileMaximumSize = 1024 * 1024 * 1024;  // 单个文件最大容量

class AsyncLogging {
 public:
  using Buffer = FixedBuffer<FixedBufferSize>;

  AsyncLogging();
  AsyncLogging(const char* filepath);
  ~AsyncLogging();

  void Start();
  void Stop();

  void Append(const char* data, int len);
  void Flush();
  
  void ThreadFunc();

 private:
  bool running_;  // 是否正在运行
  const char* filepath_;  // 文件路径

  // 缓冲区
  std::unique_ptr<Buffer> current_; // 当前的缓存
  std::unique_ptr<Buffer> next_; // 空闲的缓冲
  std::vector<std::unique_ptr<Buffer>> buffers_;// 已满的缓冲区

  // 线程相关
  std::mutex mutex_;
  std::condition_variable cv_;
  Latch latch_;
  std::thread thread_;
};
