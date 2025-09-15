#pragma once

#include "Buffer.h"
#include "Callbacks.h"
#include "Timestamp.h"
#include "noncopyable.h"
#include "InetAddress.h"
#include <atomic>
#include <cstddef>
#include <memory>
#include <string>

class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection> {
    public:
        TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &localAddr, const InetAddress &peerAddr);
        ~TcpConnection();

        EventLoop *getLoop() const {return loop_;}
        const std::string &name() const { return name_; }
        const InetAddress &localAddress() const { return localAddr_; }
        const InetAddress &peerAddress() const { return peerAddr_; }

        bool connected() const { return state_ == kConnected; }

        void send(const std::string &buf);
        void shutdown();

        void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
        void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
        void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
        void setHighWaterMarkCallback(const HighWaterMarkCallback &cb) { highWaterMarkCallback_ = cb; }
        void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

        void connectEstablished();
        void connectDestoryed();

    private:
        enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };

        void handleRead(Timestamp receiveTime);
        void handleWrite();
        void handleClose();
        void handleError();

        void setState(StateE state) {
            state_ = state;
        }

        void sendInLoop(const void *msg, size_t len);
        void shutdownInLoop();


        EventLoop *loop_;
        std::string name_;
        std::atomic_int state_;
        bool reading_;

        std::unique_ptr<Socket> socket_;
        std::unique_ptr<Channel> channel_;

        const InetAddress peerAddr_;
        const InetAddress localAddr_;

        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        CloseCallback closeCallback_;
        HighWaterMarkCallback highWaterMarkCallback_;

        size_t highWaterMark_;

        Buffer inputBuffer_;
        Buffer outputBuffer_;

};
