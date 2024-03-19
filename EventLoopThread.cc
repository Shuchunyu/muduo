 #include "EventLoopThread.h"
 #include "EventLoop.h"

 #include<memory>

 EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const std::string &name)
    :loop_(nullptr)
    ,exiting_(false)
    ,thread_(std::bind(&EventLoopThread::threadFunc, this), name)
    ,mutex_()
    ,cond_()
    ,callback_(cb)
    {

    }

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != nullptr)
    {
        loop_ -> quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::stratLoop()
{
    thread_.start();    // 启动子线程运行绑定的函数

    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr)
        {
            cond_.wait(lock);   //等待子线程创建EventLoop
        }
        loop = loop_;
    }
    return loop;
}

//下面这个方法，在单独的新线程中运行
void EventLoopThread::threadFunc()
{
    EventLoop loop;
    if(callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();            //一直循环  如果返回，说明loop被关闭了
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}