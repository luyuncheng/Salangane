//
// Created by root on 16-6-9.
//

#ifndef SALANGANE_ACCEPTOR_H
#define SALANGANE_ACCEPTOR_H

#include <functional>
#include <boost/noncopyable.hpp>

#include "../net/Channel.h"
#include "../net/Socket.h"

namespace salangane{
    namespace net {
        class EventLoop;
        class InetAddress;
        ///
/// Acceptor of incoming TCP connections.
///
        class Acceptor : boost::noncopyable {
        public:
            typedef std::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;

            Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
            ~Acceptor();

            void setNewConnectionCallback(const NewConnectionCallback& cb)
            { newConnectionCallback_ = cb; }

            bool listenning() const { return listenning_; }
            void listen();

        private:
            void handleRead();

            EventLoop* loop_;
            Socket acceptSocket_;
            Channel acceptChannel_;
            NewConnectionCallback newConnectionCallback_;
            bool listenning_;
            int idleFd_;
        };
    }
}

#endif //SALANGANE_ACCEPTOR_H
