#pragma once

#include<functional>
#include<thread>
#include<memory>
#include<unistd.h>
#include<string>
#include<atomic>

#include "noncopyable.h"

class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc, const std::string &name = std::string());
    ~Thread();
    
    void start();
    void join();

    bool started() const{return started_;}
    pid_t tid() const {return tid_;}
    const std::string& name() const {return name_;}

    static int numCreated() {return numCreated_;}

private:
    void setDefaultName();

    bool started_;
    bool joined_;
    //std::thread thread_; 如果这样定义会直接产生线程开始运行
    std::shared_ptr<std::thread> thread_;//使用智能指针控制线程产生的时机
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;
    static std::atomic_int numCreated_;
};