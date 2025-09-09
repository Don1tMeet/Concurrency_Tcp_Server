#pragma once
#include <sys/epoll.h>
#include <vector>

class Channel;
class Epoll {
 public:
  Epoll();
  Epoll(const Epoll &other) = delete;
  Epoll(Epoll &&other) = delete;
  Epoll &operator=(const Epoll &other) = delete;
  Epoll &operator=(Epoll &&other) = delete;
  ~Epoll();

  void UpdateChannel(Channel *_ch);  // 更新监听的事件
  void DeleteChannel(Channel *_ch);  // 删除监听的事件
  
  std::vector<Channel *> Poll(int timeout = -1);  // 返回就绪的事件

 private:
  int epfd_;
  struct epoll_event *events_;
};
