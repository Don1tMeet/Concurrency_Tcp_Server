#pragma once

#include "TimeStamp.h"
#include <cstdio>
#include <unistd.h>
#include <sys/time.h>
#include <string>

// flush的间隔时间
static const time_t FlushInterval = 3;

class LogFile {
 public:
  LogFile();
  LogFile(const char *filepath);
  ~LogFile();

  // 向文件写入数据
  void Write(const char *data, int len);

  // 刷新文件缓冲区
  void Flush();

  int64_t GetWrittenBytes() const;  // 返回已写入字节数

 private:
  FILE *fp_;
  int64_t written_bytes_;  // 已写入字节数
  time_t lastwrite_;  // 最后写入的时间
  time_t lastflush_;  // 最后刷新的时间
};