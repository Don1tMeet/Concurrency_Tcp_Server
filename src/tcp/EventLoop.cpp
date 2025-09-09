#include <sys/eventfd.h>
#include <unistd.h>
#include "EventLoop.h"
#include "TimerQueue.h"
#include "Channel.h"
#include "Epoll.h"
#include "ThreadPool.h"
#include "CurrentThread.h"
#include <cassert>

EventLoop::EventLoop() {
  ep_ = std::make_unique<Epoll>(); 
  wakeup_fd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);  // 创建一个eventfd，用于唤醒
  wakeup_channel_ = std::make_unique<Channel>(this, wakeup_fd_);  // 创建一个channel，用于唤醒
  calling_functors_ = false;
  timer_queue_ = std::make_unique<TimerQueue>(this);  // 创建一个定时器队列

  wakeup_channel_->SetReadCallBack(std::bind(&EventLoop::HandleRead, this));
  wakeup_channel_->EnableRead();
}

EventLoop::~EventLoop() {
  DeleteChannel(wakeup_channel_.get());  // 删除channel
  ::close(wakeup_fd_);  // 关闭fd
}

void EventLoop::Loop() {
  // 每个loop绑定在不同的线程上，在执行poll前绑定当前的线程id
  tid_ = CurrentThread::tid();
  while (!quit_) {
    for (Channel *ac_channel : ep_->Poll(-1)) {
      ac_channel->HandleEvent();
    }
    DoToDoList();
  }
}

void EventLoop::DoToDoList() {
  calling_functors_ = true;
  std::vector< std::function<void()> > functors;
  {
    // 加锁
    std::unique_lock<std::mutex> lock(mutex_);
    functors.swap(to_do_list_);
  }
  // 执行待处理的事件
  for (auto &functor : functors) {
    functor();
  }

  calling_functors_ = false;
}

void EventLoop::QueueOneFunc(std::function<void()> cb) {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    to_do_list_.emplace_back(std::move(cb));
  }

  // 因为删除连接的操作都交给main_reactor_来做，而to_do_list_在处理完poll后执行
  // 如果main_reactor_一直没有事件发生，那么就是一直阻塞在poll
  // 这样就一直不会执行to_do_list_中的事件
  // 因此在to_do_list_中添加事件后，唤醒main_reactor_

  // 如果调用该函数的不是loop绑定的线程，说明有一个连接删除了，转交给了main_reactor_来处理
  // 此时唤醒main_reactor_来处理
  // 如果调用该函数的是loop绑定的线程，且calling_functors_为true
  // 说明该线程正在处理to_do_list_中的事件，但此时又有新的事件需要处理，因此也需要唤醒main_reactor_
  // 在下一次处理这些新加入的事件
  if(!IsInLoopThread() || calling_functors_) {
    uint64_t one = 1;  // 随便写入一个数据，触发wakeup_channel_的可读事件
    ssize_t write_size = ::write(wakeup_fd_, &one, sizeof(one));
    (void) write_size;  // 避免未使用变量的警告
    assert(write_size == sizeof(one));
  }
}

void EventLoop::RunOneFunc(std::function<void()> cb) {
  if (IsInLoopThread()) {
    cb();
  } else {
    QueueOneFunc(cb);
  }
}

bool EventLoop::IsInLoopThread() {
  return CurrentThread::tid() == tid_;
}

void EventLoop::HandleRead() {
  // 读取wakeup_fd_中的数据，恢复eventfd的状态
  uint64_t one = 1;
  ssize_t read_size = ::read(wakeup_fd_, &one, sizeof(one));
  (void) read_size;  // 避免未使用变量的警告
  assert(read_size == sizeof(one));
}

void EventLoop::UpdateChannel(Channel *ch) { ep_->UpdateChannel(ch); }
void EventLoop::DeleteChannel(Channel *ch) { ep_->DeleteChannel(ch); }

void EventLoop::RunAt(TimeStamp timeStamp, std::function<void()> const &cb) {
  timer_queue_->AddTimer(timeStamp, cb, 0.0);
}

void EventLoop::RunAfter(double delay, std::function<void()> const &cb) {
  TimeStamp timeStamp = TimeStamp::AddTime(TimeStamp::Now(), delay);
  timer_queue_->AddTimer(timeStamp, cb, 0.0);
}

void EventLoop::RunEvery(double interval, std::function<void()> const &cb) {
  TimeStamp timeStamp = TimeStamp::AddTime(TimeStamp::Now(), interval);
  timer_queue_->AddTimer(timeStamp, cb, interval);
}