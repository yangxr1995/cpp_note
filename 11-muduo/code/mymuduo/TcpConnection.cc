#include "TcpConnection.h"
#include "Callbacks.h"
#include "logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include <asm-generic/socket.h>
#include <cerrno>
#include <cstddef>
#include <functional>
#include <memory>
#include <sys/types.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>



static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
        LOG_FATAL("EventLoop is not null\n");
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, 
        int sockfd, const InetAddress &localAddr, const InetAddress &peerAddr)
    :loop_(CheckLoopNotNull(loop))
     , name_(name)
     , state_(kConnecting)
     , reading_(true)
     , socket_(new Socket(sockfd))
     , channel_(new Channel(loop, sockfd))
     , localAddr_(localAddr)
     , peerAddr_(peerAddr)
     , highWaterMark_(64 * 1024 * 1024)
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));

    LOG_INFO("TcpConnection::ctor[%s] at fd = %d\n", name_.c_str(), sockfd);

    socket_->setKeepAlive(true);
}


TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnection::dtor[%s] at fd = %d state = %d", name_.c_str(), channel_->fd(), state_.load());
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
    if (n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0) {
        handleClose();
    }
    else {
        errno = saveErrno;
        LOG_ERROR("TcpConnection::handleRead\n");
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    int saveErrno = 0;
    if (channel_->isWriting()) {
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saveErrno);
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
            }
            if (state_ == kDisconnecting) {
                shutdownInLoop();
            }
        }
        else {
            LOG_ERROR("handleWrite\n");
        }
    }
    else {
        LOG_ERROR("TcpConnection fd=%d is down, no more writing\n", channel_->fd());
    }
}

void TcpConnection::handleClose()
{
    LOG_INFO("fd = %d state=%d\n", channel_->fd(), state_.load());
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr);
    closeCallback_(connPtr);
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof(optval);
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return ;
    }
    int err = optval;
    LOG_ERROR("TcpConnection::handleError %s SO_ERROR = %s\n", name_.c_str(), strerror(err));
}


void TcpConnection::sendInLoop(const void *data, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false; 

    // 之前调用过 shutdown
    if (state_ == kDisconnected) {
        LOG_ERROR("kDisconnected, give up writing\n");
        return;
    }

    // if no thing in output queue, try writing directly
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_->fd(), data , len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_) {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else { // nwrote < 0
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_ERROR("TcpConnection::sendInLoop\n");
                if (errno == EPIPE || errno == ECONNRESET) { // 对端重置连接
                    faultError = true;
                }
            }
        }
    }

    // 一次write没有把数据全部发出去，加入缓冲区，使能channel的写事件
    if (!faultError && remaining > 0) {
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_
                && oldLen < highWaterMark_
                && highWaterMarkCallback_) {
            loop_->queueInLoop(
                    std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append((char *)data + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }

}

// 不暴露给用户，保证在正确的Loop
void TcpConnection::shutdownInLoop()
{
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
    else {
        // 数据没有发送完，不处理
    }
}

// 暴露给用户调用，所以可能运行在不同thread，所以调用 runInLoop保证subthread正确
void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::send(const std::string &buf)
{
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf.c_str(), buf.size());
        }
        else {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }

}

void TcpConnection::connectEstablished()
{
    setState(kConnected);
    // TcpConnection 是暴露给用户的，用户可能在回调中删除TcpConnection，
    // 所以Channel需要监听TcpConnection是否存在
    channel_->tie(shared_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestoryed()
{
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}
