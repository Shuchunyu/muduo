#pragma once
#include"noncopyable.h"
#include"Timestamp.h"
#include<functional>

class Timer : noncopyable
{
public:
    using TimerCallback = std::function<void()>;
    // 构造函数
    Timer(TimerCallback cb, Timestamp when, double interval)
        : callback_(move(cb)),
          expiration_(when),
          interval_(interval),
          repeat_(interval > 0.0) // 一次性定时器设置为0
    {
    }
    // 调用此定时器的回调函数
    void run() const 
    { 
        callback_(); 
    }
    // 返回此定时器超时时间
    Timestamp expiration() const  { return expiration_; }
    bool repeat() const { return repeat_; }

    // 重启定时器(如果是非重复事件则到期时间置为0)
    void restart(Timestamp now);

private:
    const TimerCallback callback_;  // 定时器回调函数
    Timestamp expiration_;          // 下一次的超时时刻
    const double interval_;         // 超时时间间隔，如果是一次性定时器，该值为0
    const bool repeat_;             // 是否重复(false 表示是一次性定时器)
};