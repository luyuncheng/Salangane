//
// Created by root on 16-6-5.
//

#ifndef SALANGANE_POLLER_H
#define SALANGANE_POLLER_H

#include <map>
#include <vector>
#include <boost/noncopyable.hpp>

#include "../base/Timestamp.h"
#include "../net/EventLoop.h"
//Poller 是 PollPoller 和 EPollPoller 的基类，采用“电平触发”的语意。它是
//EventLoop 的成员，生命期由后者控制。

//PollPoller 和 EPollPoller 封装 poll(2) 和 epoll(4) 两种 IO Multiplexing 后端。
//Poll 的存在价值是便于调试，因为 poll(2) 调用是上下文无关的，用 strace(1) 很
//容易知道库的行为是否正确。
namespace salangane {
    namespace net {
        class channel;

        ///
        /// Base class for IO Multiplexing
        ///
        /// This class doesn't own the Channel objects.

        class Poller : boost::noncopyable {
        public:
            typedef std::vector<Channel*> ChannelList; //EventLoop.h typedef

            Poller(EventLoop *loop);
            virtual ~Poller();

            /// Polls the I/O events.
            /// Must be called in the loop thread.
            virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

            /// Changes the interested I/O events.
            /// Must be called in the loop thread.
            virtual void updateChannel(Channel* channel) = 0;

            /// Remove the channel, when it destructs.
            /// Must be called in the loop thread.
            virtual void removeChannel(Channel* channel) = 0;

            virtual bool hasChannel(Channel* channel) const;

            static Poller* newDefaultPoller(EventLoop* loop);

            void assertInLoopThread() const {
                ownerLoop_->assertInLoopThread();
            }
        protected:
            typedef std::map<int, Channel*> ChannelMap;
            ChannelMap channels_;
        private:
            EventLoop *ownerLoop_;
        };
    }
}


#endif //SALANGANE_POLLER_H
