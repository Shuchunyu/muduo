#include "TcpServer.h"
#include "Logger.h"
#include "TcpConnection.h"

#include<functional>
#include<string.h>


static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainloop is null!\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}


TcpServer::TcpServer(EventLoop* loop, const InetAddress &listenAddr, const std::string& nameArg, Option option)
    :loop_(CheckLoopNotNull(loop))
    ,ipPort_(listenAddr.toIpPort())
    ,name_(nameArg)
    ,acceptor_(new Acceptor(loop, listenAddr, option == kReusePort))
    ,threadPool_(new EventLoopThreadPool(loop, name_))
    ,connectionCallback_()
    ,messageCallback_()
    ,nextConnId_(1)
    ,started_(0)
{
    //有新用户连接进来时，执行TcpServer::newConnection回调
    acceptor_ -> setNewConnectionCallback(std::bind(&TcpServer::newConnection, this,
        std::placeholders::_1, std::placeholders::_2));

}

TcpServer::~TcpServer()
{
    for(auto &item: connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();

        conn -> getLoop() -> runInLoop(
            std::bind(&TcpConnection::connectDestoryed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    threadPool_ -> setThreadNum(numThreads);
}

void TcpServer::start()
{
    if(started_++ == 0)     //防止tcpserver被启动多次
    {
        threadPool_ -> start(threadInitCallback_);
        loop_ -> runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
        //baseloop的loop由用户调用
    }
}

//有新的连接，acceptor会执行这个回调
void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    EventLoop* ioloop = threadPool_ ->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s \n",
        name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

    //通过socket获取本机的ip地址和端口
    sockaddr_in local;
    ::bzero(&local, sizeof local);
    socklen_t addrlen = sizeof local;
    if(::getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0)
    {
        LOG_ERROR("sockets::getlocalAddr");
    }
    InetAddress localAddr(local);

    TcpConnectionPtr conn(new TcpConnection(
                        ioloop,
                        connName,
                        sockfd,
                        localAddr,
                        peerAddr));
    
    connections_[connName] = conn;
    //用户传给TcpServer -> TcpConnection -> Channel -> Poller -> Channel回调
    conn -> setConnectionCallback(connectionCallback_);
    conn -> setMessageCallback(messageCallback_);
    conn -> setWriteCompleteCallback(writeCompleteCallback_);
    //conn -> shutdown
    conn -> setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    
    ioloop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO("In %p TcpServer::removeConnection", loop_);
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
   LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s\n"
        ,name_.c_str(), conn->name().c_str());

    connections_.erase(conn -> name());
    EventLoop *ioloop = conn -> getLoop();
    ioloop->queueInLoop(std::bind(&TcpConnection::connectDestoryed, conn));
}