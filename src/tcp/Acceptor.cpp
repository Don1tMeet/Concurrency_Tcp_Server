#include "Acceptor.h"
#include "Channel.h"
#include "EventLoop.h"
#include "util.h"

#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <utility>
#include <memory>

Acceptor::Acceptor(EventLoop *_loop, const char * ip, const int port) : loop_(_loop), listenfd_(-1) {
  Create();
  Bind(ip, port);
  Listen();


  // sock->setnonblocking();   //接收连接不使用非阻塞socket
  accept_channel_ = std::make_unique<Channel>(loop_, listenfd_);
  std::function<void()> cb = std::bind(&Acceptor::AcceptConnection, this);
  accept_channel_->SetReadCallBack(cb);
  accept_channel_->EnableRead();
}

Acceptor::~Acceptor() {
  loop_->DeleteChannel(accept_channel_.get());
  ::close(listenfd_);
}

void Acceptor::Create() {
  assert(listenfd_ == -1);
  listenfd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  Errif(listenfd_ < 0, "socket error");
}

void Acceptor::Bind(const char *ip, const int port) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip);
  int ret = ::bind(listenfd_, (struct sockaddr *)&addr, sizeof(addr));
  Errif(ret < 0, "bind error");
}

void Acceptor::Listen() {
  assert(listenfd_ != -1);
  int ret = ::listen(listenfd_, SOMAXCONN);
  Errif(ret < 0, "listen error");
}

void Acceptor::AcceptConnection() {
  struct sockaddr_in clnt_addr;
  socklen_t clnt_addr_len = sizeof(clnt_addr);
  assert(listenfd_ != -1);
  int clnt_fd = ::accept(listenfd_, (struct sockaddr *)&clnt_addr, &clnt_addr_len);
  
  Errif(clnt_fd < 0, "accept error");

  if(new_connection_callback_) {
    new_connection_callback_(clnt_fd);
  }
}

void Acceptor::SetNewConnectionCallBack(std::function<void(int)> _cb) { new_connection_callback_ = std::move(_cb); }
