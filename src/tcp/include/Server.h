#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>

class EventLoop;
class ThreadPool;
class Acceptor;
class Connection;
class Server {
 public:
  explicit Server(const char *ip, const int port, unsigned int thread_size);
  Server(const Server &other) = delete;
  Server(Server &&other) = delete;
  Server &operator=(const Server &other) = delete;
  Server &operator=(Server &&other) = delete;
  ~Server();

  void Start();
  
  void HandleNewConnection(int fd);
  void HandleClose(const std::shared_ptr<Connection> &conn);
  // 对handleclose的封装，确保删除和增加连接都由main_reactor_来做
  void HandleCloseInLoop(const std::shared_ptr<Connection> &conn);

  void SetConnectionCallBack(std::function<void(const std::shared_ptr<Connection> &)> fn);
  void SetMessageCallBack(std::function<void(const std::shared_ptr<Connection> &)> fn);


  
  EventLoop* GetMainLoop();
  EventLoop* GetSubLoop(unsigned int index);
 private:
  std::unique_ptr<EventLoop> main_reactor_;
  std::vector<std::unique_ptr<EventLoop>> sub_reactors_;
  int next_conn_id_;

  std::unique_ptr<Acceptor> acceptor_;
  std::unordered_map<int, std::shared_ptr<Connection>> connections_;
  std::unique_ptr<ThreadPool> threadpool_;
  std::function<void(const std::shared_ptr<Connection> &)> on_connect_callback_;
  std::function<void(const std::shared_ptr<Connection> &)> on_message_callback_;
};
