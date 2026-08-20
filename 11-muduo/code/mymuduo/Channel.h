#pragma once

#include "Timestamp.h"
#include "noncopyable.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <utility>

class EventLoop;
class Timestamp;

class Channel : noncopyable {
    public:
        using EventCallback = std::function<void()>;
        using ReadEventCallback = std::function<void(Timestamp)>;

        Channel(EventLoop *loop, int fd);

        ~Channel();

        void handleEvent(Timestamp receiveTime);

        void setReadCallback(ReadEventCallback cb)
        { readCallback_ = std::move(cb); }

        void setWriteCallback(EventCallback cb)
        { writeCallback_ = std::move(cb); }

        void setCloseCallback(EventCallback cb)
        { closeCallback_ = std::move(cb); }

        void setErrorCallback(EventCallback cb)
        { errorCallback_ = std::move(cb); }

        void tie(const std::shared_ptr<void> &);

        int fd() const { return fd_; }
        int events() const { return events_; }

        void set_revents(int revents) { revents_ = revents; }

        bool isNoneEvent() const { return events_ == kNoneEvent; }

        void enableReading() { events_ |= kReadEvent; update(); }
        void enableWriting() { events_ |= kWriteEvent; update(); }
        void disableReading() { events_ &= ~kReadEvent; update(); }
        void disableWriting() { events_ &= ~kWriteEvent; update(); }
        void disableAll() { events_ = kNoneEvent; update(); }

        bool isWriting() const { return events_ & kWriteEvent; }
        bool isReading() const { return events_ & kReadEvent; }

        int index() const { return index_; }
        void set_index(int index) { index_ = index; }

        EventLoop * ownerLoop() { return loop_; }

        void remove();

    private:
        void update();
        void handleEventWithGuard(Timestamp receiveTime);

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop *loop_;
        const int fd_;
        int events_;
        int revents_;
        int index_; // Channel 的状态

        // 举个例子
        // TcpConnection->channel
        // 所以 TcpConnection 的生命周期需要大于等于 channel
        // 但是 TcpConnection 由用户持有，可能
        std::weak_ptr<void> tie_;
        bool tied_;

        ReadEventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback closeCallback_;
        EventCallback errorCallback_;

};
