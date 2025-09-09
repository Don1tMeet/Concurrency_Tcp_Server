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

  while (true) {
    Buffer *sendbuf = conn->GetSendBuffer();
    sendbuf->Clear();  // 清空发送缓冲区
    std::cout << "请输入要发送的信息：";
    std::string str;
    std::cin >> str;
    sendbuf->SetBuf(str.c_str());  // 设置发送缓冲区
    if(str == "exit") {  // 输入exit，退出
      break;
    }
    conn->Write();  // 写到服务器
    if (conn->GetState() != Connection::State::Connected) {  // 连接关闭
      std::cout << "连接关闭，退出程序" << std::endl;
      break;
    }
    conn->Read();  // 读取服务器回传的数据
    std::cout << "从服务器接收到信息：" << conn->GetReadBuffer()->Buf() << std::endl;
  }

  return 0;
}
