#pragma once

#include "Channel.h"
#include "InetAddress.h"
#include "Socket.h"
#include "noncopyable.h"
#include <functional>

class EventLoop;

class Acceptor : noncopyable {
    public:
        using NewConnectionCallback  = std::function<void(int sockfd, const InetAddress &)>;
        Acceptor(EventLoop *, const InetAddress &listenAddr, bool reuseport);
        ~Acceptor();

        void setNewConnectionCallback(const NewConnectionCallback &cb) {
            newConnectionCallback_ = cb;
        }

        bool listenning() const { return listenning_; }
        void listen();

    private:
        void handleRead();

        bool listenning_;
        NewConnectionCallback newConnectionCallback_;
        EventLoop *loop_;
        Socket acceptSocket_;
        Channel acceptChannel_;
};
