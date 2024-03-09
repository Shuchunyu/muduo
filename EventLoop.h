#pragma once

#include<functional>
#include<vector>
#include<atomic>
#include<memory>
#include<mutex>

#include "noncopyable.h"

class Channel;
class Poller;

class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    //开启事件循环
    void loop();
    //退出事件循环
    void quit();

    //Eventloop 调用 poller
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);
private:
    using ChannelList = std::vector<Channel*>;

    std::atomic_bool looping_;  //原子操作，通过CAS实现
    std::atomic_bool quit_;     //标志退出loop循环
    
    const pid_t threadId_;                      //记录loop所在线程的id
    Timestamp pollReturnTime_;                  //poller返回发生时间的channels的时间点
    std::unique_ptr<Poller> poller_;

    int wakeupFd_;                          //当mianloop获取到新用户的channel，通过轮询算法选择一个subloop，通过该成员唤醒subloop处理channel
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    //Channel* currentActiveChannel_;

    std::atomic_bool callingPendingFunctors_;   //标识当前loop是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_;      //存储loop需要执行的所有回调操作
    std::mutex mutex_;                          //互斥锁用来保护上面vector的线程安全
};