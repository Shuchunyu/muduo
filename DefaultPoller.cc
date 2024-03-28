//实现Poller中的 static Poller* newDefaultPoller(EventLoop* loop);方法，因为需要包含派生类对象，如果在基类包含，有点违反继承思想，所以单独写在一个文件中

#include "Poller.h"
#include "EPollPoller.h"

#include<stdlib.h>

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        return nullptr; //生成poll的实例
    }
    else
    {
        return new EPollPoller(loop); //生成epoll的实例
    }
}