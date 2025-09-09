#pragma once
#include <cstdint>

class InetAddress;
class Socket {
 public:
  Socket();
  explicit Socket(int fd);
  Socket(const Socket &other) = delete;
  Socket(Socket &&other) = delete;
  Socket &operator=(const Socket &other) = delete;
  Socket &operator=(Socket &&other) = delete;
  ~Socket();

  void Bind(InetAddress *addr);
  void Listen();
  int Accept(InetAddress *addr);

  void Connect(InetAddress *addr);
  void Connect(const char *ip, uint16_t port);

  int GetFd();
  void SetNonBlocking();
  bool IsNonBlocking();

 private:
  int fd_ = -1;
};
