#pragma once
#include "TimeStamp.h"
#include <functional>
#include <memory>
#include <mutex>

class Epoll;
class Channel;
class TimerQueue;

class EventLoop {
 public:
  EventLoop();
  EventLoop(const EventLoop &other) = delete;
  EventLoop(EventLoop &&other) = delete;
  EventLoop &operator=(const EventLoop &other) = delete;
  EventLoop &operator=(EventLoop &&other) = delete;
  ~EventLoop();

  void Loop();

  void DoToDoList();  // 执行待处理的事件
  void QueueOneFunc(std::function<void()> cb);  // 将cb加入到to_do_list_

  // 如果是loop绑定的线程，则立即执行cb，否则加入到to_do_list_
  // 因为如果是loop绑定的线程在执行cb，那么直接执行即可
  // 如果不是，那么loop绑定的线程可能在执行其它事件，因此将cb加入to_do_list_
  void RunOneFunc(std::function<void()> cb);

  bool IsInLoopThread();  // 判断调用该函数的是不是该loop绑定的线程

  void HandleRead();  // 用于唤醒

  void UpdateChannel(Channel *ch);  // 更新channel
  void DeleteChannel(Channel *ch);  // 删除channel

  // 定时器相关
  void RunAt(TimeStamp timeStamp, std::function<void()> const &cb);  // 在指定时间执行cb
  void RunAfter(double delay, std::function<void()> const &cb);  // 延迟delay秒执行cb
  void RunEvery(double interval, std::function<void()> const &cb);  // 每隔interval秒执行cb

 private:
  std::unique_ptr<Epoll> ep_;
  std::mutex mutex_;  // 互斥锁
  std::vector<std::function<void()>> to_do_list_;  // 待处理的事件列表
  bool quit_ = false;

  int wakeup_fd_;  // 用于唤醒的fd
  std::unique_ptr<Channel> wakeup_channel_;  // 用于唤醒的channel

  bool calling_functors_;  // 是否正在执行函数
  pid_t tid_;  // 线程id

  std::unique_ptr<TimerQueue> timer_queue_;  // 定时器队列
};
