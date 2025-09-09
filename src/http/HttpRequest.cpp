#include "HttpRequest.h"
#include <iostream>

HttpRequest::HttpRequest() : method_(kInvalid), version_(kUnknown) {}

HttpRequest::~HttpRequest() {}

bool HttpRequest::SetMethod(const std::string &method) {
  if(method == "GET") {
    method_ = Method::kGet;
  }
  else if(method == "POST") {
    method_ =  Method::kPost;
  }
  else if(method == "HEAD") {
    method_ = Method::kHead;
  }
  else if(method == "PUT") {
    method_ = Method::kPut;
  }
  else if(method == "Delete") {
    method_ = Method::kDelete;
  }
  else {
    method_ = Method::kInvalid;
  }
  return method_ != Method::kInvalid;
}

HttpRequest::Method HttpRequest::GetMethod() const { return method_; }

std::string HttpRequest::GetMethodString() const {
  std::string method_str;
  if (method_ == Method::kGet) {
    method_str = "GET";
  }
  else if (method_ == Method::kPost) {
    method_str = "POST";
  }
  else if (method_ == Method::kHead) {
    method_str =  "HEAD";
  }
  else if(method_ == Method::kPut) {
    method_str = "PUT";
  }
  else if (method_ == Method::kDelete) {
    method_str =  "DELETE";
  }else{
    method_str = "INVALID";
  }
  return method_str;
}

void HttpRequest::SetVersion(const std::string &ver) {
  if(ver == "1.0") {
    version_ = Version::kHttp10;
  }
  else if(ver == "1.1") {
    version_ = Version::kHttp11;
  }
  else {
    version_ = Version::kUnknown;
  }
}

HttpRequest::Version HttpRequest::GetVersion() const { return version_; }

std::string HttpRequest::GetVersionString() const {
  std::string ver_str;
  if(version_ == Version::kHttp10) {
    ver_str = "http1.0";
  }
  else if(version_ == Version::kHttp11) {
    ver_str = "http1.1";
  }
  else {
    ver_str = "unknown";
  }
  return ver_str;
}

void HttpRequest::SetUrl(const std::string &url) { url_ = std::move(url);}

const std::string &HttpRequest::GetUrl() const { return url_; }

void HttpRequest::SetProtocol(const std::string &str) { protocol_ = std::move(str);}

const std::string &HttpRequest::GetProtocol() const { return protocol_; }

void HttpRequest::SetRequestParams(const std::string &key, const std::string &value) {
  request_params_[key] = value;
}

std::string HttpRequest::GetRequestValue(const std::string &key) const {
  std::string ret;
  auto it = request_params_.find(key);
  return it == request_params_.end() ? ret : it->second;
}

const std::map<std::string, std::string> &HttpRequest::GetRequestParams() const {
  return request_params_;
}

void HttpRequest::AddHeader(const std::string &field, const std::string &value) {
  headers_[field] = value;
}

std::string HttpRequest::GetHeaderValue(const std::string &field) const {
  std::string ret;
  auto it = headers_.find(field);
  return it == headers_.end() ? ret : it->second;
}

const std::map<std::string, std::string> &HttpRequest::GetHeaders() const {
  return headers_;
}

void HttpRequest::SetBody(const std::string &str) {
  body_ = std::move(str);
}

const std::string &HttpRequest::GetBody() const {
  return body_;
}
