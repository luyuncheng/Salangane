//
// Created by root on 16-6-5.
//

#include "../net/Poller.h"
#include "../net/Channel.h"

using namespace salangane;
using namespace salangane::net;


Poller::Poller(EventLoop* loop)
        : ownerLoop_(loop)
{ }

Poller::~Poller()
{ }

bool Poller::hasChannel(Channel* channel) const {
    assertInLoopThread();
    ChannelMap::const_iterator it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}