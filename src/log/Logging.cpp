#include "Logging.h"
#include "CurrentThread.h"
#include <utility>


__thread char t_time[64];  // 当前线程的时间字符串 “年:月:日 时:分:秒”
__thread time_t t_lastsecond;  // 当前线程上一次日志记录时的秒数


// 方便一个已知长度的字符串被送入buffer中
class Template {
 public:
  Template(const char* str, unsigned len) : str_(str), len_(len) {}
  const char* str_;
  const unsigned len_;
};

// 重载运算符，使LogStream可以处理Template类型的数据。
inline LogStream& operator<<(LogStream& s, Template v){
  s.Append(v.str_, v.len_);
  return s;
}
// 重载运算符，使LogStream可以处理SourceFile类型的数据
inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v){
    s.Append(v.data_, v.size_);
    return s;
}



Logger::SourceFile::SourceFile(const char* data) : data_(data), size_(static_cast<int>(strlen(data_))) {
  // 找到data中最后一个'/'
  const char *last_slash = strrchr(data, '/');
  if (last_slash) {  // 只取最后一个'/'后面的内容
    data_ = last_slash + 1;
    size_ -= static_cast<int>((data_ - data));
  }
}

Logger::Impl::Impl(Logger::LogLevel level, const Logger::SourceFile &source, int line)
                  : level_(level), sourcefile_(source), line_(line) {
  // 格式化时间
  FormattedTime();
  // 初始化当前线程id
  CurrentThread::tid();

  // 输出当前线程信息和日志等级
  stream_ << Template(CurrentThread::tidString(), CurrentThread::tidStringLength());
  stream_ << Template(GetLogLevelString(), 6);
}

Logger::Impl::~Impl() {}

void Logger::Impl::FormattedTime() {
  //格式化输出时间
  TimeStamp now = TimeStamp::Now();
  time_t seconds = static_cast<time_t>(now.GetMicroSeconds() / 1000000);
  int microseconds = static_cast<int>(now.GetMicroSeconds() % 1000000);

  // 如果不在同一秒内，则更新日志记录的时间
  if(t_lastsecond != seconds){
    struct  tm tm_time;
    // 获取到达当前时间seconds的tm结构体
    localtime_r(&seconds, &tm_time);
    
    snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d.",
            tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    t_lastsecond = seconds;
  }

  // 格式化微妙并输出时间和微妙
  Fmt us(".%06dZ", microseconds);
  stream_ << Template(t_time, 17) << Template(us.Data(), 9);
}

void Logger::Impl::Finish() {
  stream_ << " - " << sourcefile_.data_ << ":" << line_ << "\n";
}

LogStream& Logger::Impl::stream() {
  return stream_;
}

const char* Logger::Impl::GetLogLevelString() const {
  switch(level_) {
    case DEBUG:
      return "DEBUF ";
    case INFO:
      return "INFO  ";
    case WARN:
      return "WARN  ";
    case ERROR:
      return "ERROR ";
    case FATAL:
      return "FATAL ";
  }
  return "UKNOWN";
}

// 默认写到stdout
void defaultOutput(const char* msg, int len) {
  fwrite(msg, 1, len, stdout);
}

// 默认flush到stdout
void defaultFlush(){
  fflush(stdout);
}

// 定义默认值
Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;
Logger::LogLevel g_logLevel = Logger::LogLevel::INFO;

Logger::Logger(const char* file_, int line, Logger::LogLevel level) : impl_(level, SourceFile(file_), line) {}

Logger::~Logger() {
  // 我们使用的使Logger的临时变量，如果析构，说明已经完成了日志的输出
  // 因此调用impl_.Finish()来补充源代码的位置和行数
  impl_.Finish();
  const LogStream::Buffer& buf(stream().GetBuffer());  // 获取缓冲区
  // 将缓冲区的内容输入到对应的输出流
  g_output(buf.Data(), buf.Size());

  // 如果日志级别为FATAL，flush设备缓冲区并终止程序
  if(impl_.level_ == FATAL) {
    g_flush();
    abort();
  }
}

LogStream &Logger::stream() {
  return impl_.stream();
}

void Logger::SetOutput(Logger::OutputFunc func){
  g_output = func;
}

void Logger::SetFlush(Logger::FlushFunc func){
  g_flush = func;
}

void Logger::SetLogLevel(Logger::LogLevel level){
  g_logLevel = level;
}