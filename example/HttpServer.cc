#include "HttpServer.h"

#include "HttpResponse.h"
#include <functional>

void defaultHttpCallback(const HttpRequest &, HttpResponse *resp)
{
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

HttpServer::HttpServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const std::string &name,
                       TcpServer::Option option = TcpServer::kNoReusePort)
    : server_(loop, listenAddr, name, option),
      httpCallback_(defaultHttpCallback)
{
    server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void HttpServer::start()
{
    std::cout << "HttpServer[" << server_.name() << "] starts listening on " << server_.ipPort();
    server_.start();
}