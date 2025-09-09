#include "Epoll.h"
#include <unistd.h>
#include <cstring>
#include "Channel.h"
#include "util.h"

#define MAX_EVENTS 1024

Epoll::Epoll() {
  epfd_ = ::epoll_create1(0);
  Errif(epfd_ == -1, "epoll create error");
  events_ = new epoll_event[MAX_EVENTS];
  memset(events_, 0, sizeof(*events_) * MAX_EVENTS);
}

Epoll::~Epoll() {
  if (epfd_ != -1) {
    close(epfd_);
    epfd_ = -1;
  }
  delete[] events_;
}

void Epoll::UpdateChannel(Channel *_ch) {
  int fd = _ch->GetFd();
  struct epoll_event ev = {};
  memset(&ev, 0, sizeof(ev));
  ev.data.ptr = _ch;
  ev.events = _ch->GetListenEvents();
  if (_ch->IsInEpoll()) {  // 已经在epoll中，更新
    Errif(epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify error");
  } else {  // 否则加入
    Errif(epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
    _ch->SetInEpoll(true);
  }
}

void Epoll::DeleteChannel(Channel *_ch) {
  int fd = _ch->GetFd();
  Errif(epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) == -1, "epoll delete error");
  _ch->SetInEpoll(false);
}

std::vector<Channel *> Epoll::Poll(int timeout) {
  std::vector<Channel *> res;
  int nfds = epoll_wait(epfd_, events_, MAX_EVENTS, timeout);
  Errif(nfds == -1, "epoll wait error");
  for (int i = 0; i != nfds; ++i) {
    Channel *ch = (Channel *)events_[i].data.ptr;
    ch->SetReadyEvents(events_[i].events);
    res.push_back(ch);
  }
  return res;
}
