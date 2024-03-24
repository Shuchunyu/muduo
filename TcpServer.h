#pragma once

#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include "EventLoopThreadPool.h"
#include "Callbacks.h"
#include "TcpConnection.h"
#include "Buffer.h"

#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>

// 对外的服务器编程使用的类
class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    enum Option
    {
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop* loop, const InetAddress &listenAddr, const std::string& nameArg, Option option = kNoReusePort);
    ~TcpServer();

    void setThreadNum(int numThreads);

    void setThreadCallback(const ThreadInitCallback& cb){threadInitCallback_ = cb;}
    void setConnectionCallback(const ConnectionCallback& cb) {connectionCallback_ = cb;}
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb;}
    void setWriteCompleteback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb;}

    void start();


private:
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;                                   //用户定义的baseloop
    const std::string ipPort_;  
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;   //one loop per thread

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;

    std::atomic_int started_;

    int nextConnId_;
    ConnectionMap connections_;

};