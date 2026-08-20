#include <cerrno>
#include <cstring>
#include <sys/epoll.h>

#include "EPollPoller.h"
#include "Poller.h"
#include "Timestamp.h"
#include "logger.h"
#include "Channel.h"

// Channel新创建，未添加到Poller中
const int kNew = -1;
// Channel已添加到Poller中
const int kAdded = 1;
// Channel已被删除
const int kDeleted = 2;

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop)
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    , events_(kInitEventListSize)
{

    if (epollfd_ < 0) {
        LOG_FATAL("epoll_create1 err:%d\n", errno);
    }
}

EPollPoller::~EPollPoller() 
{

}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    LOG_DEBUG("%s: fd => total count : %d\n", __func__, channels_.size());

    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), events_.size(), timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());

    if (numEvents > 0) {
        LOG_INFO("%d events happened \n", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        // 可能由于 events_不够长导致没有接受完所有事件，对events_ 进行扩容
        // 因为使用水平触发，所以即使没有处理完也能再次触发
        if (numEvents == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0) {
        LOG_DEBUG("%s: timeout\n", __func__);
    }
    else {
        if (savedErrno != EINTR) {
            errno = savedErrno;
            LOG_ERROR("%s: epoll_wait err : %d", __func__, errno);
        }
    }

    return now;
}

void EPollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();
    LOG_INFO("fd=%d events=%d index=%d\n", channel->fd(), channel->events(), index);
    if (index == kNew || index == kDeleted) {
        if (index == kNew) {
            int fd = channel->fd();
            channels_[fd] = channel;
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else {
        int fd = channel->fd();
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    channels_.erase(fd);

    int index = channel->index();
    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvents; ++i) {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }

}

void EPollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_ERROR("epoll_ctl del err: %s\n", strerror(errno));
        }
        else {
            LOG_FATAL("epoll_ctl add/mod err: %d\n", errno);
        }
    }
}

