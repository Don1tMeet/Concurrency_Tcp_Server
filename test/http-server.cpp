#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "EventLoop.h"
#include "Logging.h"
#include "AsyncLogging.h"
#include <iostream>
#include <string>
#include <fstream>
#include <memory>

std::string ReadFile(const std::string& path) {
  // 以只读方式打开文件
  std::ifstream fin(path.c_str(), std::ifstream::in);

  // 获取文件长度，先定位到文件末尾，再获取文件指针位置
  fin.seekg(0, fin.end);
  int flen = fin.tellg();

  // 重置文件流指针
  fin.seekg(0, fin.beg);

  // 读取文件
  char * buffer = new char[flen];
  fin.read(buffer, flen);
  std::string msg(buffer, flen);
  delete[] buffer;
  return msg;
}

// http的测试程序，主要完成对请求的处理，即HandleResponse函数
void HttpResponseCallBack(const HttpRequest &request, HttpResponse *response) {
  
  std::string url = request.GetUrl();

  // GET
  if(request.GetMethod() == HttpRequest::Method::kGet) {
    if(url == "/") {  // url为根目录
      std::string body = ReadFile("../static/index.html");
      response->SetStatusCode(HttpResponse::HttpStatusCode::k200OK);
      response->SetStatusMessage("OK");
      response->SetContentType("text/html");
      response->SetBody(body);
    }
    else if(url == "/mhw") {
      std::string body = ReadFile("../static/mhw.html");
      response->SetStatusCode(HttpResponse::HttpStatusCode::k200OK);
      response->SetStatusMessage("OK");
      response->SetContentType("text/html");
      response->SetBody(body);
    }
    else if(url == "/cat.jpg") {
      std::string body = ReadFile("../static/cat.jpg");
      response->SetStatusCode(HttpResponse::HttpStatusCode::k200OK);
      response->SetStatusMessage("OK");
      response->SetContentType("image/jpeg");
      response->SetBody(body);
    }
    else {  // 其它url，不存在
      response->SetStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
      response->SetStatusMessage("Not Found");
      response->SetBody("404 Not Found\n");
      response->SetCloseConnection(true);
    }
  }
  // POST
  else if(request.GetMethod() == HttpRequest::Method::kPost) {
    if(url == "/login") {
      // 获取登录界面（即body）
      std::string rqbody = request.GetBody();

      // 解析
      // 获取username和passwork字段的内容
      int usernamePos = rqbody.find("username=");
      int passwordPos = rqbody.find("password=");
      usernamePos += 9;  // "username="的长度，即跳转到"username="之后的第一个位置
      passwordPos += 9;

      // 找打中间分隔符
      size_t usernameEndPos = rqbody.find('&', usernamePos);
      size_t passwordEndPos = rqbody.length();

      // 获取username和password
      std::string username = rqbody.substr(usernamePos, usernameEndPos - usernameEndPos);
      std::string password = rqbody.substr(passwordPos, passwordEndPos - passwordPos);
    
      if (username == "wlgls") {
        response->SetBody("login ok!\n");
      }
      else {
        response->SetBody("error!\n");
      }

      response->SetStatusCode(HttpResponse::HttpStatusCode::k200OK);
      response->SetStatusMessage("OK");
      response->SetContentType("text/plain");
    }
  }
}

// 声明异步日志库
std::unique_ptr<AsyncLogging> asynclog;
void AsyncOupputFunc(const char* data, int len){
  asynclog->Append(data, len);
}

void AsyncFlushFunc() {
  asynclog->Flush();
}

int main(int argc, char *argv[]) {
  int port;
  if(argc <= 1) {
    port = 1234;
  }
  else if(argc == 2) {
    port = atoi(argv[1]);
  }
  else {
    std::cout << "error" << std::endl;
    exit(0);
  }

  // // 初始化异步日志库
  // asynclog = std::make_unique<AsyncLogging>();
  // Logger::SetOutput(AsyncOupputFunc);
  // Logger::SetFlush(AsyncFlushFunc);

  // asynclog->Start();

  unsigned int thread_size = std::thread::hardware_concurrency() - 1;
  HttpServer server("127.0.0.1", port, thread_size, true);  // 创建http服务器
  
  
  server.SetHttpCallBack(HttpResponseCallBack);  // 设置http响应的回调函数
  

  server.Start();  // 启动http服务器
  return 0;
}