#include "LogFile.h"
#include "TimeStamp.h"
#include <string>
#include <iostream>

LogFile::LogFile() : fp_(nullptr), written_bytes_(0), lastwrite_(0), lastflush_(0) {
  std::string defaultpath = std::move("../LogFiles/LogFile_" + 
                              TimeStamp::Now().ToDefaultLogString() +
                              ".log");
    
  fp_ = ::fopen(defaultpath.data(), "a+");
}

LogFile::LogFile(const char *filepath)
          : fp_(::fopen(filepath, "a+")), written_bytes_(0), lastwrite_(0), lastflush_(0) {
  
  // 如果文件未打开（不存在），使用默认的文件路径
  if(!fp_) {
    std::string defaultpath = std::move("../LogFiles/LogFile_" + 
                              TimeStamp::Now().ToDefaultLogString() +
                              ".log");
    
    fp_ = ::fopen(defaultpath.data(), "a+");
  }

}

LogFile::~LogFile() {
  Flush();
  if(fp_) {
    fclose(fp_);
  }
}

void LogFile::Write(const char* data, int len) {
  int pos = 0;

  while(pos != len) {
    // 使用无锁版本加快写入速度，一般一个系统只有一个后端日志系统。
    pos += static_cast<int>(fwrite_unlocked(data + pos, sizeof(char), len-pos, fp_));
  }

  // 更新lastwrite_和written_bytes
  time_t now = ::time(nullptr);
  if(len != 0) {
    lastwrite_ = now;
    written_bytes_ += len;
  }

  // 判断是否需要flush
  if(lastwrite_ - lastflush_ > FlushInterval){
    Flush();
    lastflush_ = now;
  }
}

void LogFile::Flush() {
  fflush(fp_);
}

int64_t LogFile::GetWrittenBytes() const{
  return written_bytes_;
}