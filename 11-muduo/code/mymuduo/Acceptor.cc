#include "Acceptor.h"
#include <unistd.h>
#include "InetAddress.h"
#include "logger.h"
#include <cerrno>
#include <functional>
#include <sys/socket.h>


int createNonblockingOrDie()
{
    int socketfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (socketfd < 0) {
        LOG_FATAL("socket %d\n", errno);
    }
    return socketfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
    : loop_(loop)
      , acceptSocket_(createNonblockingOrDie())
      , acceptChannel_(loop_, acceptSocket_.fd())
      , listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        }
        else {
            ::close(connfd);
        }
    }
    else {
        LOG_ERROR("accept err : %d\n", errno);
    }
}


