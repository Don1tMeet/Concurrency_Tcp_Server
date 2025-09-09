#include "Connection.h"
#include "HttpContext.h"
#include "TimeStamp.h"
#include "Logging.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <functional>
#include <iostream>
#include <utility>
#include <memory>
#include "Buffer.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "util.h"
#include <fcntl.h>

#define READ_BUFFER 1024

Connection::Connection(EventLoop *_loop, int _connfd, int _connid) : loop_(_loop), connfd_(_connfd), connid_(_connid) {
  if (loop_ != nullptr) {
    channel_ = std::make_unique<Channel>(loop_, connfd_);
    channel_->SetReadCallBack(std::bind(&Connection::HandleMessage, this));
    channel_->SetWriteCallBack(std::bind(&Connection::HandleWrite, this));
    channel_->UseET();  // 使用边沿触发
  }
  read_buffer_ = std::make_unique<Buffer>();
  send_buffer_ = std::make_unique<Buffer>();
  http_context_ = std::make_unique<HttpContext>();
}

Connection::~Connection() {
  ::close(connfd_);
}

void Connection::ConnectionEstablished() {
  state_ = State::Connected;  // 已连接
  if(channel_ == nullptr){
    return;
  }
  channel_->SetTie(shared_from_this());  // 绑定到该连接
  channel_->EnableRead();
  if (on_connect_callback_) {
      on_connect_callback_(shared_from_this());
  }
}

void Connection::ConnectionDestructor() {
  // 不在析构中执行该操作，单独处理，增加性能
  // 因为在析构前，连接已经关闭了，channel因该被删除了
  loop_->DeleteChannel(channel_.get());
}

void Connection::SetCloseCallBack(std::function<void(const std::shared_ptr<Connection> &)> const &cb) {
  on_close_callback_ = std::move(cb);
}

void Connection::SetMessageCallBack(std::function<void(const std::shared_ptr<Connection> &)> const &cb) {
  on_message_callback_ = std::move(cb);
}

void Connection::SetConnectCallBack(std::function<void(const std::shared_ptr<Connection> &)> const &cb) {
  on_connect_callback_ = std::move(cb);
}

void Connection::HandleMessage(){
  Read();
  if(on_message_callback_){
    on_message_callback_(shared_from_this());
  }
}

void Connection::HandleWrite() {
  assert(state_ == State::Connected);

  LOG_INFO << "TcpConnection::HandlWrite";

  WriteNonBlocking();

}

void Connection::HandleClose(){
  //std::cout << CurrentThread::tid() << " TcpConnection::HandleClose" << std::endl;
  if(state_ != State::Disconnected){
    state_ = State::Disconnected;
    if(on_close_callback_){
      on_close_callback_(shared_from_this());
    }
  }
}


EventLoop *Connection::GetLoop() { return loop_; }

int Connection::GetFd() { return connfd_; }

int Connection::GetId() { return connid_; }

HttpContext* Connection::GetHttpContext() {
  return http_context_.get();
}

TimeStamp Connection::GetTimeStamp() const {
  return last_active_time_;
}

void Connection::UpdateTimeStamp(TimeStamp now) {
  last_active_time_ = now;
}

Connection::State Connection::GetState() { return state_; }

Buffer *Connection::GetReadBuffer() { return read_buffer_.get(); }

Buffer *Connection::GetSendBuffer() { return send_buffer_.get(); }

void Connection::Send(const char *msg, int len) {
  int remaining = len;  // 未发送数据
  int send_bytes = 0;  // 已发送数据

  // 如果send_buffer_没有可读数据，说明此时的数据为最新数据，直接发送
  if(send_buffer_->ReadAbleBytes() == 0) {
    send_bytes = static_cast<int>(write(connfd_, msg, len));

    if(send_bytes >= 0) {  // 成功发送了数据，但可能未发完
      remaining -= send_bytes;
    }
    else if(send_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      // 说明此时TCP缓冲区是满的，无法写入数据，什么都不做
      send_bytes = 0;
    }
    else {  // 错误
      LOG_ERROR << "TcpConnection::Send - TcpConnection Send ERROR";
      return;
    }
  }

  // 将剩余数据加入到send_buffer，等待后续发送
  assert(remaining <= len);
  if(remaining > 0) {
    send_buffer_->Append(msg + send_bytes, remaining);

    // 如果此前没有注册监听写时间，那么在此时监听
    // 如果已经监听了并且触发了，那么此时再次监听，强制触发一次
    // 如果强制触发失败，任然可以等待后续TCP缓冲区可写
    channel_->EnableWrite();
  }
}

void Connection::Send(const char *msg) {
  Send(msg, static_cast<int>(strlen(msg)));
}

void Connection::Send(std::string const &msg) {
  Send(msg.data(), static_cast<int>(msg.size()));
}

void Connection::Read() {
  assert(state_ == State::Connected);
  read_buffer_->RetrieveAll();  // 读之前清空缓冲区
  if (fcntl(connfd_, F_GETFL) & O_NONBLOCK) {  // 非阻塞
    ReadNonBlocking();
  } else {  // 阻塞
    ReadBlocking();
  }
}

void Connection::Write() {
  assert(state_ == State::Connected);
  if (fcntl(connfd_, F_GETFL) & O_NONBLOCK) {  // 非阻塞
    WriteNonBlocking();
  } else {  // 阻塞
    WriteBlocking();
  }
  send_buffer_->RetrieveAll();  // 写完后清空缓冲区
}

void Connection::ReadNonBlocking() {
  char buf[READ_BUFFER];
  while (true) {
    memset(buf, 0, READ_BUFFER);
    ssize_t read_bytes = read(connfd_, buf, READ_BUFFER);
    if (read_bytes > 0) {
      read_buffer_->Append(buf, read_bytes);
    } else if (read_bytes == -1 && errno == EINTR) {  // 正常中断
      // std::cout << "continue reading." << std::endl;
      continue;
    } else if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {  // 读取完毕
      break;
    } else if (read_bytes == 0) {  // 客户端断开连接
      // std::cout << "read nonblocking error: client fd: " << connfd_ << " disconnected." << std::endl;
      HandleClose();
      break;
    } else {  // 其它错误
      // std::cout << "read nonblocking error: client fd: " << connfd_ << " other error." << std::endl;
      HandleClose();
      break;
    }
  }
}

void Connection::WriteNonBlocking() {
  int remaining = send_buffer_->ReadAbleBytes();
  int send_byres= static_cast<int>(write(connfd_, send_buffer_->Peek(), remaining));

  if(send_byres == -1 && (errno == EAGAIN|| errno == EWOULDBLOCK)) {
    // 说明此时TCP缓冲区是满的，没有办法写入，什么都不做
    // 主要是防止，在Send时write后监听EPOLLOUT，但是TCP缓冲区还是满的
    send_byres = 0;
  }
  else if(send_byres == -1) {
    LOG_ERROR << "TcpConnection::Send - TcpConnection Send ERROR";
  }

  remaining -= send_byres;
  send_buffer_->Retrieve(send_byres);
}

void Connection::ReadBlocking() {
  unsigned int rcv_size = 0;
  socklen_t len = sizeof(rcv_size);
  getsockopt(connfd_, SOL_SOCKET, SO_RCVBUF, &rcv_size, &len);  // 获取接收缓冲区大小
  char buf[rcv_size];
  memset(buf, 0, rcv_size);
  ssize_t read_bytes = read(connfd_, buf, rcv_size);
  if (read_bytes > 0) {
    read_buffer_->Append(buf, read_bytes);
  } else if (read_bytes == 0) {
    // std::cout << "read blocking error: client fd: " << connfd_ << " disconnected." << std::endl;
    HandleClose();
  } else {
    // std::cout << "read blocking error: client fd: " << connfd_ << " other error." << std::endl;
    HandleClose();
  }
}

void Connection::WriteBlocking() {
  // 没有解决send_buffer_->Size() > 发送缓冲区的问题，可能会有bug
  // unsigned int snd_size = 0;
  // socklen_t len = sizeof(snd_size);
  // getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &snd_size, &len);  // 获取发送缓冲区大小
  ssize_t write_bytes = write(connfd_, send_buffer_->BeginRead(), send_buffer_->ReadAbleBytes());
  if (write_bytes == -1) {
    // std::cout << "write blocking error: client fd: " << connfd_ << " other error." << std::endl;
    HandleClose();
  }
}
