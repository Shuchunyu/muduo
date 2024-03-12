#include "EventLoop.h"
#include "Poller.h"
#include "Channel.h"
#include "Logger.h"

#include<sys/eventfd.h>

__thread EventLoop* t_loopInThisThread = nullptr;

//创建eventfd，用来notify唤醒subReator处理新来的channel
int createEventfd()
{
    int evfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evfd < 0)
    {
        LOG_FATAL("eventfd error: %d\n", errno);
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
    //, currentActiveChannel_(nullptr)
{
    LOG_DEBUG("EventLoop created %p in thread %d\n", this, threadId_);
    if(t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread %d \n", t_loopInThisThread, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }

    //设置wakerup的事件类型以及发生事件后的回调操作
    wakeupChannel_ -> setReadCallback(std::bind(&EventLoop::handleRead, this));
    //每一个eventloop都将监听wakeupChannel的EPOLLIN读事件了
    wakeupChannel_ -> enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_-> disableAll();
    wakeupChannel_ -> remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::handleRead() read %ld bytes instead of 8", n);
    }
}

void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start looping\n", this);

    while(!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_ -> poll(kPollTimeMs, &activeChannels_);

        for(Channel* channel: activeChannels_)
        {
            //Poller监听哪些channel发生事件了，然后上报给Eventloop，通知channel处理事件
            channel -> handleEvent(pollReturnTime_);
        }
        
        //事件循环需要处理的回调操作
        //mainloop 事先注册一个回调cb，唤醒subloop后，执行下面的方法，执行之前mainloop注册的回调
        doPendingFunctors();
    }

    LOG_INFO("EventLoop %p stop looping\n", this);
}

void EventLoop::quit()
{
    quit_ = true;
    if(!isInLoopThread()) //如果在其他线程中，调用了quit, 需要唤醒loop所在那个线程
    {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    
}

void EventLoop::queueInLoop(Functor cb)
{
    

}

//唤醒loop。想wakeupfd写数据，使poller返回
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8\n", n);
    }
}


void EventLoop::updateChannel(Channel* channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors()
{
    
} 