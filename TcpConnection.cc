#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

#include <functional>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>

static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainloop is null!\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop
                , const std::string& name
                , int sockfd
                , const InetAddress& localAddr
                , const InetAddress& peerAddr)
    : loop_(CheckLoopNotNull(loop))
    , name_(name)
    , state_(kConnecting)
    , reading_(true)
    , socket_(new Socket(sockfd))
    , channel_(new Channel(loop_, sockfd))
    , localAddr_(localAddr)
    , peerAddr_(peerAddr)
    , highWaterMark_(64 * 1024 * 1024)      //64MB
{
    channel_ -> setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_ -> setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_ -> setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_ -> setErrorCallback(std::bind(&TcpConnection::handleError, this));

    LOG_INFO("TcpConnection::constructor[%s] fd=%d\n", name_.c_str(), sockfd);
    socket_->setKeepAlive(true);
}
    
TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnection::destructor[%s] fd=%d\n", name_.c_str(), channel_ -> fd());
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_ -> fd(), &savedErrno);
    if(n > 0)
    {
        //已建立连接的用户，有可读事件发生了，调用用户传入的onMessage函数
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if(n == 0)
    {
        handleClose();
    }
    else
    {
        errno = savedErrno;
        LOG_ERROR("TcpConnection:handleRead\n");
        handleError();
    }

}

void TcpConnection::handleWrite()
{
    if(channel_->isWriting())   // 如果channel注册了写事件
    {
        int savedErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &savedErrno);     //写数据

        if(n > 0)
        {
            outputBuffer_.retrieve(n);                                      //写完数据后移动buffer中的指针
            if(outputBuffer_.readableBytes() == 0)                          //输出缓冲区没有数据可以读。就是缓冲区里没有数据等待发送
            {
                channel_ -> disableWriting();
                if(writeCompleteCallback_)
                {
                    loop_ -> queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if(state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            LOG_ERROR("TcpConnection::handleWrite");
        }
    }
    else
    {
        LOG_ERROR("TcpConnection fd=%d is down, no more writing\n", channel_ -> fd());
    }
}

void TcpConnection::handleClose()
{
    LOG_INFO("handle close fd=%d state=%d\n", channel_-> fd(), (int)state_);
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr);   //连接关闭了，调用用户传递的关于连接的回调
    closeCallback_(connPtr);        //关闭连接的回调

}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen =  sizeof optval;
    int err = 0;
    if(::getsockopt(channel_ -> fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcpConnecton::handlerError name:%s - SO_ERROR:%d\n", name_.c_str(), err);
}

void TcpConnection::send(const std::string& buf)
{
    if(state_ == kConnected)
    {
        if(loop_ -> isInLoopThread())
        {
            sendInLoop(buf.c_str(), buf.size());
        }
        else
        {
            loop_->runInLoop(
                std::bind(&TcpConnection::sendInLoop,
                this,
                buf.c_str(),
                buf.size())
            );
        }

    }
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
    size_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if(state_ == kDisconnected)
    {
        LOG_ERROR("Disconnected, give up writing!\n");
        return;
    }

    //第一次开始写数据，而且缓冲区中没有待发送内容
    if(!channel_ -> isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), data, len);
        if(nwrote > 0)
        {
            remaining = len - nwrote;
            if(remaining == 0 && writeCompleteCallback_)
            {
                loop_-> queueInLoop(
                    std::bind(writeCompleteCallback_, shared_from_this())
                );
            }
        }
        else
        {
            nwrote = 0;
            if(errno != EWOULDBLOCK) //不是因为正常的非阻塞返回错误
            {
                LOG_ERROR("TcpConnection::sendInLoop");
                if(errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
    }
    
    //
    if(!faultError && remaining > 0)
    {
        size_t oldlen = outputBuffer_.readableBytes();
        if(oldlen + remaining >= highWaterMark_
            && oldlen < highWaterMark_
            && highWaterMarkCallback_)
        {
            loop_->queueInLoop(
                std::bind(highWaterMarkCallback_, shared_from_this(), oldlen + remaining)
            );
        }
        outputBuffer_.append((char*)data + nwrote, remaining);
        if(!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());  //channel的弱智能指针指向tcpconnecton，执行回调时，检查tcpconnection是否还存在
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestoryed()
{
    if(state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::shutdown()
{
    if(state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    if(!channel_->isWriting())  //说明当前channel中的数据全部发送完成
    {
        socket_->shutdownWrite();   //会触发socket的EPOLLHUP事件，这个事件是默认注册的
    }
}