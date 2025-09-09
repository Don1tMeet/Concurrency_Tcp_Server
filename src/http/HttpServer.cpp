#include "HttpServer.h"
#include "Server.h"
#include "HttpRequest.h"
#include "HttpContext.h"
#include "HttpResponse.h"
#include "TimeStamp.h"
#include "Connection.h"
#include "EventLoop.h"
#include "CurrentThread.h"
#include "Buffer.h"
#include <arpa/inet.h>
#include <functional>
#include <iostream>
#include "Logging.h"

void HttpServer::HttpDefaultCallBack(const HttpRequest &request, HttpResponse *resp) {
  resp->SetStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
  resp->SetStatusMessage("Not Found");
  resp->SetCloseConnection(true);
}

HttpServer::HttpServer(const char *ip, const int port, unsigned int thread_size, bool auto_close_conn) : auto_close_conn_(auto_close_conn) {
  server_ = std::make_unique<Server>(ip, port, thread_size);
  // 设置连接创建时的回调函数
  server_->SetConnectionCallBack(
      std::bind(&HttpServer::HandleConnection, this, std::placeholders::_1));
  // 设置受到消息时的回调函数
  server_->SetMessageCallBack(
      std::bind(&HttpServer::HandleMessage, this, std::placeholders::_1));
  // 设置http响应的回调函数
  SetHttpCallBack(
    std::bind(&HttpServer::HttpDefaultCallBack, this, std::placeholders::_1, std::placeholders::_2));

  // 输出日志信息
  LOG_INFO << "HttpServer Listening on [ " << ip << ":" << port << " ]";
}

HttpServer::~HttpServer() {}

void HttpServer::Start() {
  server_->Start();
}

void HttpServer::SetHttpCallBack(const HttpResponseCallBack &cb) {
  response_callback_ = std::move(cb);
}

void HttpServer::HandleConnection(const ConnectionPtr &conn) {
  int clnt_fd = conn->GetFd();
  struct sockaddr_in peeraddr;
  socklen_t peeraddr_len = sizeof(peeraddr);
  getpeername(clnt_fd, (struct sockaddr *)&peeraddr, &peeraddr_len);

  // 输出日志信息
  LOG_INFO << " HttpServer::HandleConnection: Add connection"
           << "[ fd#" << clnt_fd << "-id#" << conn->GetId() << " ]"
           << " from" << inet_ntoa(peeraddr.sin_addr) << ":" << ntohs(peeraddr.sin_port);

  if(auto_close_conn_) {  // 如果设置了自动关闭连接
    // 获取该连接绑定的loop并设置定时任务
    conn->GetLoop()->RunAfter(AUTOCLOSETIMEOUT, std::bind(&HttpServer::ActiveCloseConn, this, std::weak_ptr<Connection>(conn)));
  }
}

void HttpServer::HandleMessage(const ConnectionPtr &conn) {
  if(conn->GetState() == Connection::State::Connected) {
    // 更新活跃时间（如果设置了自动关闭连接）
    if(auto_close_conn_) {
      conn->UpdateTimeStamp(TimeStamp::Now());
    }


    // 处理http请求
    HttpContext *context = conn->GetHttpContext();  // 获取该连接的http上下文
    // 解析http请求
    bool parse_result = context->ParaseRequest(conn->GetReadBuffer()->BeginRead(), conn->GetReadBuffer()->ReadAbleBytes());
    
    if(!parse_result) {  // 无法解析
      // 发送响应后，关闭连接
      conn->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
      conn->HandleClose();
    }

    if(context->IsCompleteRequest()) {  // 请求解析完成
      HandleRequest(conn, *context->GetRequest());  // 处理请求
      context->ResetContextStatus();  // 重置请求状态
    }
  }
}

void HttpServer::HandleRequest(const ConnectionPtr &conn, const HttpRequest &request) {
  std::string connection_state = request.GetHeaderValue("Connection");  // 获取连接状态
  // 如果连接设置为close或http版本为1.0，且为设置keep-alive，则设置连接状态为关闭
  bool close = connection_state == "close" ||
              (request.GetVersion() == HttpRequest::Version::kHttp10 && connection_state != "keep-alive");

  // 创建响应对象
  HttpResponse response(close);
  response_callback_(request, &response);  // 调用响应回调函数，设置响应

  // 发送响应
  conn->Send(response.GetMessage().c_str());

  if(response.IsCloseConnection()) {  // 如果连接状态为close，则发送响应后关闭连接
    conn->HandleClose();
  }
}

void HttpServer::ActiveCloseConn(std::weak_ptr<Connection> &conn) {
  ConnectionPtr shared_conn = conn.lock();  // 将conn提升为shared_ptr，判断连接是否有效
  if(shared_conn) {  // 连接未关闭
    if(TimeStamp::AddTime(shared_conn->GetTimeStamp(), AUTOCLOSETIMEOUT) < TimeStamp::Now()){
      // 如果连接的最后活跃时间+AUTOCLOSETIMEOUT小于当前时间，说明超时
      shared_conn->HandleClose();  // 关闭连接
    }
    else {
      // 如果未超时，则重新设置定时器
      shared_conn->GetLoop()->RunAfter(AUTOCLOSETIMEOUT, std::bind(&HttpServer::ActiveCloseConn, this, conn));
    }
  }
}



EventLoop* HttpServer::GetMainLoop() {
  return server_->GetMainLoop();
}
EventLoop* HttpServer::GetSubLoop(int index) {
  return server_->GetSubLoop(index);
}
