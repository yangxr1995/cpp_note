#include "Socket.h"
#include "logger.h"
#include <linux/tcp.h>

#include <asm-generic/socket.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

Socket::~Socket()
{
    close(sockfd_);
}

void Socket::bindAddress(const InetAddress &localaddr)
{
    if (0 != ::bind(sockfd_, (struct sockaddr *)localaddr.getSockAddr(), sizeof(struct sockaddr_in))) {
        LOG_FATAL("bind Socketfd : %d fail\n", sockfd_);
    }
}

void Socket::listen()
{
    if (0 != ::listen(sockfd_, 4))
        LOG_FATAL("listen : %d\n", errno);
}

int Socket::accept(InetAddress *peeraddr)
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t len = sizeof(addr);
    int connfd = ::accept(sockfd_ , (struct sockaddr *)&addr , &len);
    if (connfd >= 0) {
        peeraddr->setSockAddr(addr);
    }
    return  connfd;
}

void Socket::shutdownWrite()
{
    if (::shutdown(sockfd_, SHUT_WR)) { // 触发 EPOLLHUP
        LOG_ERROR("shutdownWrite : %d\n", errno);
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY , &optval, sizeof(optval));
}
void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR , &optval, sizeof(optval));
}
void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT , &optval, sizeof(optval));
}
void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE , &optval, sizeof(optval));
}


