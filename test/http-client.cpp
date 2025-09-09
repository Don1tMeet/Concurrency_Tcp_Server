#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <memory>
#include "Buffer.h"
#include "Connection.h"
#include "InetAddress.h"
#include "util.h"

#define BUFFER_SIZE 1024
int main() {
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  Errif(sock == -1, "socket create error");
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(1234);  // 端口号
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  Errif(::connect(sock, (sockaddr *)&addr, sizeof(addr)), "socket connect error");
  
  std::shared_ptr<Connection> conn = std::make_shared<Connection>(nullptr, sock, 1);  // 创建到服务器的连接

  conn->ConnectionEstablished();
  while(true) {
    // std::string request = "GET /cat.jpg HTTP/1.1\r\nConnection: close\r\n\r\n";
    std::string request = "POST /login HTTP/1.1\r\nConnection: close\r\n\r\nusername=aaa&password=123\r\n";
    char c=0;
    std::cin >> c;

    
    conn->Send(request);  // 发送请求
    if (conn->GetState() != Connection::State::Connected) {  // 连接关闭
      std::cout << "连接关闭，退出程序" << std::endl;
      return 0;
    }
    conn->Read();  // 读取服务器回传的数据
    std::cout << "从服务器接收到信息：" << conn->GetReadBuffer()->PeekAllAsString() << std::endl;
  }
  return 0;
}