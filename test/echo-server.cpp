#include <iostream>
#include <memory>
#include "Connection.h"
#include "EventLoop.h"
#include "Server.h"
#include "Buffer.h"

int main() {
  Server *server = new Server("127.0.0.1", 1234);

  server->SetMessageCallBack([](const std::shared_ptr<Connection> &conn) {
    std::cout << "Message from client " << conn->GetId() << " is " << conn->GetReadBuffer()->CStr() << std::endl;
    if(conn->GetState() == Connection::State::Connected){
      conn->Send(conn->GetReadBuffer()->CStr());
    }
   });
   
  server->Start();

  delete server;
  return 0;
}
