#pragma once
#include "TimeStamp.h"
#include <functional>

class Timer {
 public:
  Timer(TimeStamp timestamp, std::function<void()> const &cb, double interval);
  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;
  Timer(Timer&&) = delete;
  Timer& operator=(Timer&&) = delete;
  ~Timer();

  void ReStart(TimeStamp now);  // 重启定时器（将定时器到期时间设置为now + interval）
  void Run() const;  // 执行定时器的回调函数

  TimeStamp Expiration() const;  // 获取定时器到期的绝对时间
  bool Repeat() const;  // 获取定时器是否重复

 private:
  TimeStamp expiration_;  // 定时器到期的绝对时间
  std::function<void()> callback_;  // 定时器到期时的回调函数

  double interval_;  // 定时器的间隔时间
  bool repeat_;  // 是否重复定时器
};