#pragma once
#include <string>
#include <map>

class HttpRequest {
 public:
  enum Method {
    kInvalid = 0,
    kGet,
    kPost,
    kHead,
    kPut,
    kDelete
  };
  enum Version {
    kUnknown = 0,
    kHttp10,
    kHttp11
  };

  HttpRequest();
  ~HttpRequest();

  // 请求方法
  bool SetMethod(const std::string &method);  // 设置请求方法
  Method GetMethod() const;  // 获取请求方法
  std::string GetMethodString() const;  // 获取请求方法字符串
  
  // 协议版本
  void SetVersion(const std::string &ver); // http版本
  Version GetVersion() const;
  std::string GetVersionString() const;

  // url
  void SetUrl(const std::string &url); // 请求路径
  const std::string &GetUrl() const;

  // 协议
  void SetProtocol(const std::string &str);
  const std::string & GetProtocol() const;

  // 请求参数
  void SetRequestParams(const std::string &key, const std::string &value);
  std::string GetRequestValue(const std::string &key) const;
  const std::map<std::string, std::string> & GetRequestParams() const;

  // 请求头
  void AddHeader(const std::string &field, const std::string &value); // 添加请求体
  std::string GetHeaderValue(const std::string &field) const;
  const std::map<std::string, std::string> & GetHeaders() const;

  // 请求体
  void SetBody(const std::string &str);
  const std::string & GetBody() const;

 private:
  Method method_;  // 请求方法
  Version version_;  // http版本
  std::string url_;  // 请求路径
  std::string protocol_;  // 协议
  std::map<std::string, std::string> request_params_;  // 请求参数
  std::map<std::string, std::string> headers_;  // 请求头
  std::string body_;  // 请求体
};
