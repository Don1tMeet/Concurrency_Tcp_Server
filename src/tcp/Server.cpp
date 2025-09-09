#include "Server.h"
#include <unistd.h>
#include <cstring>
#include <functional>
#include <iostream>
#include <vector>
#include "Acceptor.h"
#include "Connection.h"
#include "EventLoop.h"
#include "ThreadPool.h"
#include "util.h"
#include "CurrentThread.h"
#include <cassert>
#include "Logging.h"

#define READ_BUFFER 1024

Server::Server(const char *ip, const int port, unsigned int thread_size) : next_conn_id_(1) {
  main_reactor_ = std::make_unique<EventLoop>();
  acceptor_ = std::make_unique<Acceptor>(main_reactor_.get(), ip, port);
  std::function<void(int)> cb = std::bind(&Server::HandleNewConnection, this, std::placeholders::_1);
  acceptor_->SetNewConnectionCallBack(cb);

  unsigned int size = thread_size;
  threadpool_ = std::make_unique<ThreadPool>(size);

  for (unsigned int i = 0; i != size; ++i) {
    std::unique_ptr<EventLoop> sub_reactor = std::make_unique<EventLoop>();
    sub_reactors_.push_back(std::move(sub_reactor));  // 每个子reactor对应一个事件循环
  }

}

Server::~Server() {
}

void Server::Start() {
  for (unsigned int i = 0; i != sub_reactors_.size(); ++i) {  // 启动每个子事件循环
    std::function<void()> sub_pool = std::bind(&EventLoop::Loop, sub_reactors_[i].get());
    threadpool_->Add(std::move(sub_pool));
  }
  main_reactor_->Loop();  // 启动主事件循环
}

void Server::HandleNewConnection(int fd) {
  Errif(fd == -1, "socket connection error");

  uint64_t random = fd % sub_reactors_.size();  // 调度策略，随机
  std::shared_ptr<Connection> conn = std::make_shared<Connection>(sub_reactors_[random].get(), fd, next_conn_id_);

  std::function<void(const std::shared_ptr<Connection> &)> cb = std::bind(&Server::HandleClose, this, std::placeholders::_1);
  
  conn->SetCloseCallBack(cb);
  conn->SetMessageCallBack(on_message_callback_);
  conn->SetConnectCallBack(on_connect_callback_);
                    
  connections_[fd] = conn;

  // 分配id
  ++next_conn_id_;
  if (next_conn_id_ == 1000) {
    next_conn_id_ = 1;
  }

  conn->ConnectionEstablished();  // 连接建立
}

void Server::HandleClose(const std::shared_ptr<Connection> &conn) {
  // 关闭连接，会将连接从connections_中删除，但此时可能有新连接到来
  // 由于unordered_map不是线程安全的，所以需要加锁，或者将删除操作交给main_reactor_来做
  // 这样删除和增加操作由同一个线程执行，是串行的
  std::cout <<  CurrentThread::tid() << " TcpServer::HandleClose"  << std::endl;

  main_reactor_->RunOneFunc(std::bind(&Server::HandleCloseInLoop, this, conn));
}

void Server::HandleCloseInLoop(const std::shared_ptr<Connection> &conn) {
  // 输出日志信息
  LOG_INFO << "Server::HandleCloseInLoop - Remove connection [#id:" << conn->GetId() << "-fd#" << conn->GetFd() << "]";

  int fd = conn->GetFd();
  auto it = connections_.find(fd);
  assert(it != connections_.end());
  connections_.erase(fd);

  // 获取该连接绑定的事件循环
  EventLoop *loop = conn->GetLoop();
  // 将ConnectionDestructor加入到该事件循环的to_do_list_
  loop->QueueOneFunc(std::bind(&Connection::ConnectionDestructor, conn));

}

void Server::SetConnectionCallBack(std::function<void(const std::shared_ptr<Connection> &)> fn) {
  on_connect_callback_ = std::move(fn);
}

void Server::SetMessageCallBack(std::function<void(const std::shared_ptr<Connection> &)> fn) {
  on_message_callback_ = std::move(fn);
}



EventLoop* Server::GetMainLoop() {
  return main_reactor_.get();
}
EventLoop* Server::GetSubLoop(unsigned int index) {
  if (index >= sub_reactors_.size()) {
    return nullptr;
  }
  return sub_reactors_[index].get();
}