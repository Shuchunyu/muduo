#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"
#include<iostream>

#include<sys/epoll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop * loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false) 
{ 
}

Channel::~Channel()
{

}

//什么时候调用此方法？？        新建一个tcp连接时
void Channel::tie(const std::shared_ptr<void>& obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::update()
{
    //通过Channel所属的loop，调用Poller的方法，注册fd的events事件
    loop_ -> updateChannel(this);
}

//在Channel所属的EventLoop中删除当前Channel
void Channel::remove()
{  
    loop_ -> removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
    if(tied_)
    {
        std::shared_ptr<void> guard = tie_.lock();  // 判断这个tcpconnection是否存在
        if(guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else    //没有tie，说明是wakeupfd，或者是acceptor，tie指向的是tcpconnection
    {
        handleEventWithGuard(receiveTime);
    }    
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if(closeCallback_)
        {
            closeCallback_();
        }
    }

    if(revents_ & EPOLLERR)
    {
        if(errorCallback_)
        {
            errorCallback_();
        }
    }

    if(revents_ & (EPOLLIN | EPOLLPRI))
    {
        if(readCallback_)
        {
            readCallback_(receiveTime);
        }
    }

    if(revents_ & EPOLLOUT)
    {
        if(writeCallback_)
        {
            writeCallback_();
        }
    }

}

