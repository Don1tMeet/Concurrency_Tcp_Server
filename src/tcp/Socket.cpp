#include "Socket.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include "InetAddress.h"
#include "util.h"

Socket::Socket() {
  fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
  Errif(fd_ == -1, "socket create error");
}

Socket::Socket(int fd) : fd_(fd) { Errif(fd == -1, "socket create error"); }

Socket::~Socket() {
  if (fd_ != -1) {
    ::close(fd_);
    fd_ = -1;
  }
}

void Socket::Bind(InetAddress *addr) {
  struct sockaddr_in tmp_addr = addr->GetAddr();
  Errif(::bind(fd_, (sockaddr *)&tmp_addr, sizeof(tmp_addr)), "socket bind error");
}

void Socket::Listen() { Errif(::listen(fd_, SOMAXCONN), "socket listen error"); }

int Socket::Accept(InetAddress *addr) {
  // 用于服务端
  int clnt_sockfd = -1;
  struct sockaddr_in tmp_addr {};
  socklen_t addr_len = sizeof(tmp_addr);
  if (IsNonBlocking()) {  // 非阻塞
    while (true) {
      clnt_sockfd = ::accept(fd_, (sockaddr *)&tmp_addr, &addr_len);
      if (clnt_sockfd == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {  // 没有连接
        // continue;
      } else if (clnt_sockfd == -1) {  // 连接错误
        Errif(true, "socket accept error");
      } else {  // 连接成功
        break;
      }
    }
  } else {  // 阻塞
    clnt_sockfd = ::accept(fd_, (sockaddr *)&tmp_addr, &addr_len);
    Errif(clnt_sockfd == -1, "socket accept error");
  }
  addr->SetAddr(tmp_addr);
  return clnt_sockfd;
}

void Socket::Connect(InetAddress *addr) {
  // 用于客户端
  struct sockaddr_in tmp_addr = addr->GetAddr();
  if (IsNonBlocking()) {  // 非阻塞
    while (true) {
      int ret = ::connect(fd_, (sockaddr *)&tmp_addr, sizeof(tmp_addr));
      if (ret == 0) {  // 连接成功
        break;
      }
      if (ret == -1 && errno == EINPROGRESS) {  // 正在连接
        // continue;
        /* 连接非阻塞式sockfd建议的做法：
            The socket is nonblocking and the connection cannot be
          completed immediately.  (UNIX domain sockets failed with
          EAGAIN instead.)  It is possible to select(2) or poll(2)
          for completion by selecting the socket for writing.  After
          select(2) indicates writability, use getsockopt(2) to read
          the SO_ERROR option at level SOL_SOCKET to determine
          whether connect() completed successfully (SO_ERROR is
          zero) or unsuccessfully (SO_ERROR is one of the usual
          error codes listed here, explaining the reason for the
          failure).
          这里为了简单、不断连接直到连接完成，相当于阻塞式
          */
      } else {  // 连接错误
        Errif(true, "socket connect error");
      }
    }
  } else {  // 阻塞
    Errif(::connect(fd_, (sockaddr *)&tmp_addr, sizeof(tmp_addr)), "socket connect error");
  }
}

void Socket::Connect(const char *ip, uint16_t port) {
  InetAddress addr(ip, port);
  Connect(&addr);
}

int Socket::GetFd() { return fd_; }

void Socket::SetNonBlocking() { fcntl(fd_, F_SETFL, fcntl(fd_, F_GETFL) | O_NONBLOCK); }

bool Socket::IsNonBlocking() { return (fcntl(fd_, F_GETFL) & O_NONBLOCK); }
