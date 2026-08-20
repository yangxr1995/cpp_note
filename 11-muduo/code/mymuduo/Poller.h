#pragma once

#include "Timestamp.h"
#include <unordered_map>
#include <vector>

class Channel;
class EventLoop;

class Poller {
    public:
        using ChannelList = std::vector<Channel *>;

        Poller(EventLoop *loop);
        virtual ~Poller() = default;

        virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
        virtual void updateChannel(Channel *channel) = 0;
        virtual void removeChannel(Channel *channel) = 0;

        bool hasChannel(Channel *channel) const;

        static Poller *newDefaultPoller(EventLoop *loop);

    protected:
        using ChannelMap = std::unordered_map<int , Channel *>;
        ChannelMap channels_; // for hasChannel

    private:
        EventLoop *ownerLoop_;
};
