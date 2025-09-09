#pragma once

#include <cstring>
#include "LogStream.h"
#include "TimeStamp.h"


class Logger {
 public:
  enum LogLevel {
    DEBUG,    // 指出细粒度信息事件对调试应用程序是非常有帮助的（开发过程中使用）
    INFO,     // 表明消息在粗粒度级别上突出强调应用程序的运行过程。
    WARN,     // 系统能正常运行，但可能会出现潜在错误的情形
    ERROR,    // 指出虽然发生错误事件，但仍然不影响系统的继续运行。
    FATAL     // 指出每个严重的错误事件将会导致应用程序的退出。
  };

  // 编译器计算源文件名
  class SourceFile{
   public:
    SourceFile(const char *data);
    const char *data_;
    int size_;
  };

  // 构造函数，主要用于构造Impl
  Logger(const char *file, int line, LogLevel level);
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(Logger&&) = delete;
  ~Logger();

  // 用于日志宏，返回Impl的输出流
  LogStream& stream();

  // 全局方法，设置日志全局日志级别，flush输出目的地
  static LogLevel GetLogLevel();
  static void SetLogLevel(LogLevel level);

  // 定义函数指针
  typedef void (*OutputFunc)(const char *data, int len);
  typedef void (*FlushFunc)();
  // 默认fwrite到stdout
  static void SetOutput(OutputFunc);
  // 默认fflush到stdout
  static void SetFlush(FlushFunc);

 private:
  // 私有类，对日志消息进行封装
  class Impl {
   public:
    using LogLevel = Logger::LogLevel;

    Impl(LogLevel level, const SourceFile &source, int line);
    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;
    Impl(Impl&&) = delete;
    Impl& operator=(Impl&&) = delete;
    ~Impl();

    void FormattedTime();  // 格式化时间信息
    void Finish();  // 完成格式化，并补充输出源文件和源码位置

    LogStream& stream();
    const char* GetLogLevelString() const;  // 获取loglevel的字符串
    LogLevel level_;  // 日志级别

   private:
    Logger::SourceFile sourcefile_;  // 源代码名称
    int line_;  // 源代码行数
    LogStream stream_;  // 日志缓存流
  };

  Impl impl_;
};

// 全局的日志级别，静态成员函数定义，静态成员函数实现
extern Logger::LogLevel g_logLevel;
inline Logger::LogLevel Logger::GetLogLevel() {
  return g_logLevel;
}

// 日志宏
#define LOG_DEBUG if (Logger::GetLogLevel() <= Logger::DEBUG) \
    Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Logger::GetLogLevel() <= Logger::INFO) \
    Logger(__FILE__, __LINE__, Logger::INFO).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()