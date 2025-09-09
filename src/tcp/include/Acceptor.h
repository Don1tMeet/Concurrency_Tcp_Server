#pragma once
#include <functional>
#include <memory>

class EventLoop;
class Channel;
class Acceptor {
 public:
  explicit Acceptor(EventLoop *_loop, const char * ip, const int port);
  Acceptor(const Acceptor &) = delete;
  Acceptor(Acceptor &&) = delete;
  Acceptor &operator=(const Acceptor &) = delete;
  Acceptor &operator=(Acceptor &&) = delete;
  ~Acceptor();
  
  void SetNewConnectionCallBack(std::function<void(int)> _cb);  // 设置新连接的回调函数
  
  // 创建socket
  void Create();

  // 绑定ip和端口
  void Bind(const char *ip, const int port);

  // 监听
  void Listen();
  
  void AcceptConnection();  // 接收连接

 private:
  EventLoop *loop_;
  int listenfd_;
  std::unique_ptr<Channel> accept_channel_;
  std::function<void(int)> new_connection_callback_;  // 发生新连接执行的回调函数
};
