#pragma once

#include "noncopyable.h"
#include "Channel.h"

class EventLoop : noncopyable
{
public:
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
private:
};