#pragma once

/*
HTTP/1.1 200 OK
Date: Fri, 22 May 2009 06:07:21 GMT
Content-Type: text/html; charset=UTF-8
空行
<html>
      <head></head>
      <body>
            <!--body goes here-->
      </body>
</html>
*/
#include <map>
#include "Buffer.h"

class HttpResponse
{
public:
    enum HttpStatusCode
    {
        kUnknown,
        k200Ok = 200,
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        k404NotFound = 404,
    };

    explicit HttpResponse(bool close)
        : statusCode_(kUnknown),
          closeConnection_(close)
    {
    }

    void setStatusCode(HttpStatusCode code)
    {
        statusCode_ = code;
    }

    void setStatusMessage(const std::string &message)
    {
        statusMessage_ = message;
    }

    void setContentType(const std::string &contentType)
    {
        addHeader("Content-Type", contentType);
    }

    void addHeader(const std::string &key, const std::string &value)
    {
        headers_[key] = value;
    }

    void setBody(const std::string &body)
    {
        body_ = body;
    }

    void appendToBuffer(Buffer* output) const;
private:
    std::map<std::string, std::string> headers_;
    HttpStatusCode statusCode_;
    std::string statusMessage_;
    bool closeConnection_;
    std::string body_;
};