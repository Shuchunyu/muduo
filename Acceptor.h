#pragma once
#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

#include <functional>

class EventLoop;
class InetAddress;

class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
    
    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {
        newConnectionCallback_ = cb;
    }

    bool listenning() const{return listenning_;}
    void listen();
    
private:
    void handleRead();                              // acceptor可读时的回调，调用才方法说明有新连接进来了

    EventLoop *loop_;                               // acceptor所在的loop就是主线程loop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;   // 有TcpServer注册
    bool listenning_;

};