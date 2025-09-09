#pragma once
#include <cstdio>
#include <sys/time.h>
#include <string>


class TimeStamp {
 public:
  TimeStamp();
  explicit TimeStamp(int64_t micro_seconds);
  ~TimeStamp();

  bool operator<(const TimeStamp& rhs) const;
  bool operator>(const TimeStamp& ths) const;
  bool operator<=(const TimeStamp& ths) const;
  bool operator>=(const TimeStamp& ths) const;
  bool operator==(const TimeStamp& ths) const;

  std::string ToDefaultLogString() const;  // 将时间戳转换为字符串（用于记录日志）

  std::string ToString() const;  // 将时间戳转换为字符串

  int64_t GetMicroSeconds() const;  // 获取时间戳的微秒数

  static TimeStamp Now();  // 获取当前时间戳
  static TimeStamp AddTime(TimeStamp timestamp, double add_seconds);  // 获取指定时间戳加上指定秒数后的时间戳
 
 private:
  int64_t micro_seconds_;  // 微秒数
};