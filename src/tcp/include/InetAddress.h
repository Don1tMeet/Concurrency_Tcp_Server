#pragma once
#include <arpa/inet.h>

class InetAddress {
 public:
  InetAddress();
  InetAddress(const char *ip, uint16_t port);
  InetAddress(const InetAddress &other) = delete;
  InetAddress(InetAddress &&other) = delete;
  InetAddress &operator=(const InetAddress &other) = delete;
  InetAddress &operator=(InetAddress &&other) = delete;
  ~InetAddress() = default;

  void SetAddr(sockaddr_in addr);
  sockaddr_in GetAddr();
  const char *GetIp();
  uint16_t GetPort();

 private:
  struct sockaddr_in addr_ {};
};
