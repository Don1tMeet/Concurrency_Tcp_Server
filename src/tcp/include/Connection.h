#pragma once
#include <functional>
#include <string>
#include <memory>
#include "TimeStamp.h"

class EventLoop;
class Channel;
class Buffer;
class HttpContext;
class Connection : public std::enable_shared_from_this<Connection> {
 public:
  enum State {
    Invalid = 1,
    Handshaking,
    Connected,
    Disconnected,
    Failed,
  };

  Connection(EventLoop *_loop, int connfd, int _connid);
  Connection(const Connection &other) = delete;
  Connection(Connection &&other) = delete;
  Connection &operator=(const Connection &other) = delete;
  Connection &operator=(Connection &&other) = delete;
  ~Connection();

  void ConnectionEstablished();  // 连接建立
  void ConnectionDestructor();  // 连接析构（将channel移除）
  // 关闭时的回调函数
  void SetCloseCallBack(std::function<void(const std::shared_ptr<Connection> &)> const &cb);
  // 接收到消息的回调函数
  void SetMessageCallBack(std::function<void(const std::shared_ptr<Connection> &)> const &cb);
  // 连接建立时的回调函数
  void SetConnectCallBack(std::function<void(const std::shared_ptr<Connection> &)> const &cb);

  Buffer *GetReadBuffer();  // 返回读缓冲区
  Buffer *GetSendBuffer();  // 返回写缓冲区
  
  void Read();
  void Write();
  void Send(const char *msg, int len);
  void Send(const char *msg);
  void Send(std::string const &msg);  // 输出信息

  void HandleMessage();  // 接收到消息时，进行回调
  void HandleWrite();  // 写操作
  void HandleClose();  // 关闭连接时，进行回调

  
  State GetState();  // 获取当前状态
  EventLoop *GetLoop();  // 获取当前事件循环
  int GetFd();  // 获取该连接的socketfd
  int GetId();  // 获取连接的ID
  HttpContext* GetHttpContext();  // 获取http_context_

  TimeStamp GetTimeStamp() const;  // 获取时间戳（最后的活跃时间）
  void UpdateTimeStamp(TimeStamp now);  // 更新时间戳（最后的活跃时间）

 private:
  EventLoop *loop_;
  int connfd_;
  int connid_;
  State state_ = Invalid;

  std::unique_ptr<Channel> channel_;
  std::unique_ptr<Buffer> read_buffer_;
  std::unique_ptr<Buffer> send_buffer_;

  std::function<void(const std::shared_ptr<Connection> &)> on_close_callback_;
  std::function<void(const std::shared_ptr<Connection> &)> on_message_callback_;
  std::function<void(const std::shared_ptr<Connection> &)> on_connect_callback_;

  std::unique_ptr<HttpContext> http_context_;  // http上下文

  TimeStamp last_active_time_;  // 上次活跃时间

  void ReadNonBlocking();
  void WriteNonBlocking();
  void ReadBlocking();
  void WriteBlocking();
};
