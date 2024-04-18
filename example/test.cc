#include <mymuduo/TcpServer.h>
#include <mymuduo/Logger.h>

#include <string>
#include <functional>
#include <iostream>

class EchoServer
{
public:
    EchoServer(EventLoop* loop, const InetAddress &addr, const std::string& name)
        :loop_(loop)
        ,server_(loop, addr, name)
    {
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this,
                                    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        server_.setThreadNum(4);
    }

    void start()
    {
        server_.start();
    }
private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        if(conn->connected())
        {
            LOG_INFO("connection established : %s", conn->peerAddress().toIpPort().c_str());
        }
        else
        {
            LOG_INFO("connection destroyed : %s", conn->peerAddress().toIpPort().c_str());
        }
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
    {
        //LOG_INFO("onMessage~~~~~~~~~");
        std::string msg = buf -> retrieveAllAsString();
        //std::cout << " ------ " << msg << " ------" << std::endl;
        conn->send(msg);
        conn->shutdown();
    }

    EventLoop *loop_;
    TcpServer server_;
};


int main()
{
    //std::cout << "回声服务器，启动！！！" << std::endl;
    EventLoop loop;
    InetAddress addr(9999);
    EchoServer echoServer(&loop, addr, "EchoServer");
    echoServer.start();
    loop.loop();

    return 0;
}
