#include "Channel.h"
#include <unistd.h>
#include <utility>
#include "EventLoop.h"

Channel::Channel(EventLoop *_loop, int _fd) : loop_(_loop), fd_(_fd), 
                                              listen_events_(0), ready_events_(0),
                                              in_epoll_(false) {}

Channel::~Channel() {
}

void Channel::HandleEvent() {
  if(!tie_.expired()){
    std::shared_ptr<void> guard = tie_.lock();  // 增加tie_指向内存的引用计数
    HandleEventWithGuard();
  }
  else{
    HandleEventWithGuard();
  }
}

void Channel::HandleEventWithGuard() {
  if (ready_events_ & (EPOLLIN | EPOLLPRI)) {
    if(read_callback_) {
      read_callback_();
    }
  }
  if (ready_events_ & EPOLLOUT) {
    if(read_callback_) {
      read_callback_();
    }
  }
}

void Channel::EnableRead() {
  listen_events_ |= EPOLLIN | EPOLLPRI;
  loop_->UpdateChannel(this);
}

void Channel::EnableWrite(){
  listen_events_ |= EPOLLOUT;
  loop_->UpdateChannel(this);
}


void Channel::UseET() {
  listen_events_ |= EPOLLET;
  loop_->UpdateChannel(this);
}

int Channel::GetFd() { return fd_; }

uint32_t Channel::GetListenEvents() { return listen_events_; }

uint32_t Channel::GetReadyEvents() { return ready_events_; }

bool Channel::IsInEpoll() { return in_epoll_; }

void Channel::SetInEpoll(bool _in) { in_epoll_ = _in; }

void Channel::SetListenEvents(uint32_t _ev) { listen_events_ = _ev; }

void Channel::SetReadyEvents(uint32_t _ev) { ready_events_ = _ev; }

void Channel::SetReadCallBack(std::function<void()> _cb) { read_callback_ = std::move(_cb); }

void Channel::SetWriteCallBack(std::function<void()> _cb) { write_callback_ = std::move(_cb); }

void Channel::SetTie(const std::shared_ptr<void> &_tie) { tie_ = _tie; }  // 设置tie_
