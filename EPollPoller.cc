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