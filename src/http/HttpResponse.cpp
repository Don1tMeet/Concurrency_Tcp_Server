#include "HttpRequest.h"
#include <string>
#include "HttpResponse.h"

HttpResponse::HttpResponse(bool close_connection) : 
  close_connection_(close_connection), status_code_(HttpStatusCode::kUnknown) {}

HttpResponse::~HttpResponse() {}

void HttpResponse::SetStatusCode(HttpStatusCode status_code) {
  status_code_ = status_code;
}

void HttpResponse::SetStatusMessage(const std::string &status_message) {
  status_message_ = status_message;
}

void HttpResponse::SetCloseConnection(bool close_connection) {
  close_connection_ = close_connection;
}

void HttpResponse::SetContentType(const std::string &content_type) {
  headers_["Content-Type"] = content_type;
}

void HttpResponse::AddHeader(const std::string &key, const std::string &value) {
  headers_[key] = value;
}

void HttpResponse::SetBody(const std::string &body) {
  body_ = body;
}

bool HttpResponse::IsCloseConnection() {
  return close_connection_;
}

std::string HttpResponse::GetMessage() {
  std::string message;
  // 添加状态行
  message += "HTTP/1.1 " + std::to_string(status_code_) + " " +
            status_message_ + "\r\n";
  
  
  if(close_connection_) {  // 如果需要关闭连接，添加Connection: close头部
    message += "Connection: close\r\n";
  }
  else {  // 否则添加Connection: keep-alive头部
    message += "Content-Length: " + std::to_string(body_.size()) + "\r\n";
    message += "Connection: keep-alive\r\n";
  }

  // 添加其他头部
  for(const auto &header : headers_) {
    message += header.first + ": " + header.second + "\r\n";
  }

  // 添加空行
  message += "\r\n";

  // 添加响应体
  message += body_;

  return message;
}
