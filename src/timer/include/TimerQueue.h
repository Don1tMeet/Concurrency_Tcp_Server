#pragma once
#include "TimeStamp.h"
#include <unistd.h>
#include <sys/timerfd.h>

#include <set>
#include <vector>
#include <memory>
#include <functional>
#include <utility>

class Timer;
class EventLoop;
class Channel;

class TimerQueue {
 public:
  TimerQueue(EventLoop *loop);
  TimerQueue(const TimerQueue &) = delete;
  TimerQueue &operator=(const TimerQueue &) = delete;
  TimerQueue(TimerQueue &&) = delete;
  TimerQueue &operator=(TimerQueue &&) = delete;
  ~TimerQueue();

  void CreateTimerFd();  // 创建timerfd

  void ReadTimerFd();  // 读取timerfd事件
  void HandleRead();  // timerfd可读事件的处理函数

  void ResetTimerFd(Timer *timer);  // 重置timerfd超时时间，关注新的超时任务
  void ResetTimers();  // 将具有重复属性的定时器重新加入队列

  bool Insert(Timer *timer);  // 将定时任务加入队列
  void AddTimer(TimeStamp timeStamp, std::function<void()> const &cb, double interval);  // 添加定时任务
 
 private:
  using Entry = std::pair<TimeStamp, Timer *>;

  EventLoop *loop_;  // 事件循环
  int timerfd_;  // 定时器文件描述符
  std::unique_ptr<Channel> timerfd_channel_;  // 定时器channel

  std::set<Entry> timers_;  // 定时器集合
  std::vector<Entry> active_timers_;  // 定时器集合
};
