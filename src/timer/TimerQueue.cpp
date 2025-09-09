#include "Timer.h"
#include "TimeStamp.h"
#include "TimerQueue.h"
#include "Channel.h"
#include "EventLoop.h"
#include "util.h"
#include <cstring>
#include <sys/timerfd.h>
#include <assert.h>
#include <iostream>

// extern const int kMicrosecond2Second;

TimerQueue::TimerQueue(EventLoop *loop) : loop_(loop) {
  CreateTimerFd();
  timerfd_channel_ = std::make_unique<Channel>(loop_, timerfd_);
  timerfd_channel_->SetReadCallBack(std::bind(&TimerQueue::HandleRead, this));
  timerfd_channel_->EnableRead();
}

TimerQueue::~TimerQueue() {
  loop_->DeleteChannel(timerfd_channel_.get());
  close(timerfd_);
  for(const auto &entry : timers_) {
    delete entry.second;
  }
}

void TimerQueue::CreateTimerFd() {
  timerfd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  Errif(timerfd_ < 0, "timerfd_create");
}

void TimerQueue::ReadTimerFd() {
  uint64_t read_byte;
  ssize_t readn = ::read(timerfd_, &read_byte, sizeof(read_byte));
  if(readn != sizeof(read_byte)) {
    std::cerr << "TimerQueue::ReadTimerFd() read error" << std::endl;
  }
}

void TimerQueue::HandleRead() {
  ReadTimerFd();  // 读取timerfd事件，即将timerfd变为不可读，防止loop陷入忙碌状态
  active_timers_.clear();  // 清空（之前）激活的定时器集合

  // 获取小于当前时间的所有定时器，即已经到期的定时器
  auto end = timers_.lower_bound(Entry(TimeStamp::Now(), reinterpret_cast<Timer *>(UINTPTR_MAX)));
  // 将到期的定时器加入到active_timers_中
  active_timers_.insert(active_timers_.end(), timers_.begin(), end);
  timers_.erase(timers_.begin(), end);  // 删除到期的定时器

  // 执行到期的定时器的回调函数
  for(const auto &entry : active_timers_) {
    entry.second->Run();
  }

  // 有些定时器可能有重复属性，因此将它们加入到timers_中，并重新设置超时时间
  ResetTimers();
}

void TimerQueue::ResetTimerFd(Timer *timer) {
  struct itimerspec new_value;
  struct itimerspec old_value;
  memset(&new_value, 0, sizeof(new_value));
  memset(&old_value, 0, sizeof(old_value));

  // 定时器的到期时间还有多久（与当前时间相比）
  int64_t micro_seconds_diff = timer->Expiration().GetMicroSeconds() - TimeStamp::Now().GetMicroSeconds();
  if(micro_seconds_diff < 100) {  // 如果小于100微秒，则设置为100
    micro_seconds_diff = 100;
  }

  // 设置timerfd的超时时间
  new_value.it_value.tv_sec = static_cast<time_t>(micro_seconds_diff / 1000000);
  new_value.it_value.tv_nsec = static_cast<long>((micro_seconds_diff % 1000000) * 1000);
  // 重新设置timerfd_的超时时间，采用相对时间
  int ret = ::timerfd_settime(timerfd_, 0, &new_value, &old_value);

  Errif(ret == -1, "timerfd_settime error");
}

void TimerQueue::ResetTimers() {
  // 遍历所有已激活定时器
  for (auto &entry : active_timers_) {
    if((entry.second)->Repeat()) {  // 如果设置了可重复属性，则重置后加入到timers_
      auto timer = entry.second;
      timer->ReStart(TimeStamp::Now());  // 重启定时器（到期时间设置为当前时间 + 间隔时间）
      Insert(timer);  // 将定时器加入到timers_中
    }
    else {  // 没有设置，这从active_timers_中删除
      delete entry.second;
    }
  }

  if(!timers_.empty()) {  // 如果timers_不为空，说明还有定时任务
    ResetTimerFd(timers_.begin()->second);  // 重置第一个（最早超时的）timerfd的超时时间
  }
}

bool TimerQueue::Insert(Timer *timer) {
  bool earliest = false;  // 是否是最早超时的定时器
  // 如果timers_为空或超时时间小于第一个定时任务，则是最早超时的定时器
  if(timers_.empty() || timer->Expiration() < timers_.begin()->first) {
    earliest = true;
  }
  // 将定时器加入到timers_中
  timers_.emplace(std::move(Entry(timer->Expiration(), timer)));
  
  return earliest;
}

void TimerQueue::AddTimer(TimeStamp timestamp, std::function<void()> const &cb, double interval) {
  Timer *timer = new Timer(timestamp, cb, interval);

  if(Insert(timer)) {  // 如果是最早超时的定时器，则重置timerfd_的超时时间
    ResetTimerFd(timer);
  }
}
