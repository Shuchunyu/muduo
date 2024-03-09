#include "EPollPoller.h"
#include "Channel.h"
#include "Logger.h"

#include<errno.h>
#include<unistd.h>
#include<strings.h>


EPollPoller::EPollPoller(EventLoop* loop)
    : Poller(loop)
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    , events_(KInitEventListSize)
{
    if(epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error:%d \n", errno);
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
     //此处可用LOG_DEBUG，此函数频繁调用，频繁输出会影响性能
    LOG_INFO("func=%s => fd total count:%lu \n", __FUNCTION__, channels_.size());

    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    if(numEvents > 0)
    {
        LOG_INFO("%d events happened \n", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if(numEvents == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if(numEvents == 0)
    {
        LOG_DEBUG("%s timeout!\n", __FUNCTION__);
    }
    else
    {
        if(saveErrno != EINTR)
        {
            errno = saveErrno;
            LOG_ERROR("EPollPoller::poll() err!");
        }
    }

    return now;
}

void EPollPoller::updateChannel(Channel* channel)
{

}

void EPollPoller::removeChannel(Channel* channel)
{

}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    
}

void EPollPoller::update(int operation, Channel* channel)
{

}