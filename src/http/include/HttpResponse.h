#pragma once

#include <string>
#include <map>
#include <utility>

class HttpResponse {
 public:
  // http响应状态码
  enum HttpStatusCode {
    kUnknown = 1,
    k100Continue = 100,
    k200OK = 200,
    k400BadRequest = 400,
    k403Forbidden = 403,
    k404NotFound = 404,
    k500InternalServerError = 500,
  };

  HttpResponse(bool close_connection);
  ~HttpResponse();

  void SetStatusCode(HttpStatusCode status_code);  // 设置状态码
  void SetStatusMessage(const std::string &status_message);  // 设置状态描述
  void SetCloseConnection(bool close_connection);  // 设置是否关闭连接

  void SetContentType(const std::string &content_type);  // 设置内容类型
  void AddHeader(const std::string &key, const std::string &value);  // 添加响应头

  void SetBody(const std::string &body);  // 设置响应体

  bool IsCloseConnection();  // 获取close_connection_
  
  std::string GetMessage();  // 获取响应消息（字符串）

 private:
  bool close_connection_;  // 是否关闭连接
  HttpStatusCode status_code_;  // 状态码
  std::string status_message_;  // 状态描述
  std::map<std::string, std::string> headers_;  // 响应头
  std::string body_;  // 响应体
};