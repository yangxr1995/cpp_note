#include <cerrno>
#include <cstdint>
#include <mutex>
#include <unistd.h>
#include <functional>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <utility>
#include <vector>

#include "EventLoop.h"
#include "CurrentThread.h"
#include "logger.h"
#include "Poller.h"
#include "Channel.h"

__thread EventLoop *t_loopInThisThread =  nullptr;

const int kPollTimeMs = 10000; // 10s


int createEventfd()
{
    int evfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evfd < 0) {
        LOG_FATAL("eventfd :%d\n", errno);
    }
    return evfd;
}

EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , callingPendingFunctors_(false)
    , threadId_(CurrentThread::tid())
    , poller_(Poller::newDefaultPoller(this))
    , wakeupFd_(createEventfd())
    , wakeupChannel_(new Channel(this, wakeupFd_))
    , currentActiveChannel_(nullptr)
{
    LOG_DEBUG("EventLoop created %p in  thread %d\n", this, threadId_);
    if (t_loopInThisThread)
        LOG_FATAL("Another EventLoop %p exists in this thread %d\n", t_loopInThisThread, threadId_);
    else
        t_loopInThisThread = this;

    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if (n != sizeof(one))
        LOG_ERROR("%s read err : ", __func__, errno);
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    LOG_INFO("EventLoop %p start looping\n", this);

    while (!quit_) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for (Channel * channel  : activeChannels_) {
            channel->handleEvent(pollReturnTime_);
        }
        // 执行当前 EventLoop 需要处理的回调操作
        // mainLoop给subLoop传递的事件时，仅仅通过 channel->handleEvent 唤醒subLoop,
        // subLoop唤醒后，需要调用 doPendingFunctors 执行收到的事件
        doPendingFunctors();
    }

    LOG_INFO("EventLoop %p stop looping\n", this);
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;

    // 让其他线程的 EventLoop 退出时，由于该 EventLoop可能在 poll 阻塞中，
    // 所以需要唤醒他
    if (!isInLoopThread())
        wakeup();
}

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
        cb();
    else
        queueInLoop(cb);
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }

    // 需要 wakeup的两种情况
    // 1. 线程调用非所属 EventLoop 的 queueInLoop，需要唤醒相关线程处理cb
    // 2. 线程在处理 pendingFunctors_ 时，给自己的 EventLoop 添加 cb，需要跳过一次poll阻塞
    if (!isInLoopThread() || callingPendingFunctors_)
        wakeup();
}


void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
        LOG_FATAL("EventLoop: wakeup write %lu bytes instead of 8\n", n);
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor & functor : functors) {
        functor();
    }

    callingPendingFunctors_ = false;
}
