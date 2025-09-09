#pragma once
#include <functional>
#include <memory>
#include <cstdio>

// 自动关闭时间，单位秒
#define AUTOCLOSETIMEOUT 100

class Server;
class Connection;
class EventLoop;
class HttpRequest;
class HttpResponse;
class TimeStamp;

class HttpServer {
 public:
  using ConnectionPtr = std::shared_ptr<Connection>;
  using HttpResponseCallBack = std::function<void(const HttpRequest&, HttpResponse *)>;

  HttpServer(const char *ip, const int port, unsigned int thread_size, bool auto_close_conn);
  HttpServer(const HttpServer &) = delete;
  HttpServer &operator=(const HttpServer &) = delete;
  HttpServer(HttpServer &&) = delete;
  HttpServer &operator=(HttpServer &&) = delete;
  ~HttpServer();

  void Start();

  // 默认的http响应回调函数
  void HttpDefaultCallBack(const HttpRequest &request, HttpResponse *resp);
  // 设置http响应的回调函数
  void SetHttpCallBack(const HttpResponseCallBack &cb);

  void HandleConnection(const ConnectionPtr &conn);  // 处理新连接到来的函数
  void HandleMessage(const ConnectionPtr &conn);  // 处理消息到来的函数
  void HandleRequest(const ConnectionPtr &conn, const HttpRequest &request);  // 处理请求的函数

  // 自动关闭连接时的处理函数
  void ActiveCloseConn(std::weak_ptr<Connection> &conn);


  EventLoop* GetMainLoop();
  EventLoop* GetSubLoop(int index);
 private:
  std::unique_ptr<Server> server_;

  bool auto_close_conn_;  // 是否自动关闭连接
  // http响应的回调函数，对请求（参数1）进行处理，响应结果存放在响应（参数2）中
  HttpResponseCallBack response_callback_;
};