#pragma once
#include <sys/epoll.h>
#include <functional>
#include <memory>

class EventLoop;
class Channel {
 public:
  Channel(EventLoop *_loop, int _fd);
  Channel(const Channel &other) = delete;
  Channel(Channel &&other) = delete;
  Channel &operator=(const Channel &other) = delete;
  Channel &operator=(Channel &&other) = delete;
  ~Channel();

  void HandleEvent();  // 处理事件
  void HandleEventWithGuard();  // 处理事件（防止内存泄漏）
  void EnableRead();  // 可读
  void EnableWrite();  // 可写
  void UseET();  // 使用边沿触发

  int GetFd();

  uint32_t GetListenEvents();  // 获取监听的事件
  uint32_t GetReadyEvents();  // 获取就绪的事件
  void SetListenEvents(uint32_t _ev);
  void SetReadyEvents(uint32_t _ev);
  
  bool IsInEpoll();  // 该channel是否在epoll中
  void SetInEpoll(bool _in = true);


  void SetReadCallBack(std::function<void()> _cb);
  void SetWriteCallBack(std::function<void()> _cb);

  void SetTie(const std::shared_ptr<void> &_tie);  // 设置tie_

 private:
  EventLoop *loop_;
  int fd_;
  uint32_t listen_events_;
  uint32_t ready_events_;
  bool in_epoll_{false};
  std::function<void()> read_callback_;
  std::function<void()> write_callback_;

  std::weak_ptr<void> tie_;
};
