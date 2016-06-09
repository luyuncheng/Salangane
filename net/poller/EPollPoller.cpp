//
// Created by root on 16-6-8.
//


#include "EPollPoller.h"

#include "../../base/Logging.h"
#include "../Channel.h"

#include <boost/static_assert.hpp>

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>

using namespace salangane;
using namespace salangane::net;

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
//#define POLLIN		0x001		/* There is data to read.  */
//#define POLLPRI		0x002		/* There is urgent data to read.  */
//#define POLLOUT		0x004		/* Writing now will not block.  */
//#if defined __USE_XOPEN || defined __USE_XOPEN2K8
///* These values are defined in XPG4.2.  */
//# define POLLRDNORM	0x040		/* Normal data may be read.  */
//# define POLLRDBAND	0x080		/* Priority data may be read.  */
//# define POLLWRNORM	0x100		/* Writing now will not block.  */
//# define POLLWRBAND	0x200		/* Priority data may be written.  */
//#endif
//
//#ifdef __USE_GNU
///* These are extensions for Linux.  */
//# define POLLMSG	0x400
//# define POLLREMOVE	0x1000
//# define POLLRDHUP	0x2000
//#endif
//
///* Event types always implicitly polled for.  These bits need not be set in
//   `events', but they will appear in `revents' to indicate the status of
//   the file descriptor.  */
//#define POLLERR		0x008		/* Error condition.  */
//#define POLLHUP		0x010		/* Hung up.  */
//#define POLLNVAL	0x020		/* Invalid polling request.  */
BOOST_STATIC_ASSERT(EPOLLIN == POLLIN);
BOOST_STATIC_ASSERT(EPOLLPRI == POLLPRI);
BOOST_STATIC_ASSERT(EPOLLOUT == POLLOUT);
BOOST_STATIC_ASSERT(EPOLLRDHUP == POLLRDHUP);
BOOST_STATIC_ASSERT(EPOLLERR == POLLERR);
BOOST_STATIC_ASSERT(EPOLLHUP == POLLHUP);
// similar like static for namespace avoid val multi use in different file
namespace {
    const int kNew = -1;
    const int kAdded = 1;
    const int kDeleted = 2;
}

EPollPoller::EPollPoller(EventLoop* loop)
        : Poller(loop),
          epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
          events_(kInitEventListSize)
{
    if (epollfd_ < 0) {
        LOG_SYSFATAL << "EPollPoller::EPollPoller";
    }
}

EPollPoller::~EPollPoller() {
    ::close(epollfd_);
}
//当fd就绪了后，就把就绪的加入到activeChannels的事件队列去，ChannelMap为映射模块
Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    LOG_TRACE << "fd total count " << channels_.size();
//  typedef std::vector<struct epoll_event> EventList;
//  EventList events_;
//  epoll_wait()获得就绪文件描述符 返回一个代表就绪描述符数量的值
    int numEvents = ::epoll_wait(epollfd_,
                                 &*events_.begin(),
                                 static_cast<int>(events_.size()),
                                 timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        LOG_TRACE << numEvents << " events happended";
        fillActiveChannels(numEvents, activeChannels);
        if (implicit_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size()*2);
        }
    } else if (numEvents == 0) {
        LOG_TRACE << "nothing happended";
    } else {
        // error happens, log uncommon ones
        if (savedErrno != EINTR) {
            errno = savedErrno;
            LOG_SYSERR << "EPollPoller::poll()";
        }
    }
    return now;
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
    assert(implicit_cast<size_t>(numEvents) <= events_.size());
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
#endif
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}
//#define EPOLL_CTL_ADD 1	/* Add a file decriptor to the interface.  */
//#define EPOLL_CTL_DEL 2	/* Remove a file decriptor from the interface.  */
//#define EPOLL_CTL_MOD 3	/* Change file decriptor epoll_event structure.  */
void EPollPoller::updateChannel(Channel* channel) {
    Poller::assertInLoopThread();
    const int index = channel->index();
    LOG_TRACE << "fd = " << channel->fd()
              << " events = " << channel->events() << " index = " << index;
    if (index == kNew || index == kDeleted) {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (index == kNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        } else { // index == kDeleted
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}
void EPollPoller::removeChannel(Channel* channel) {
    Poller::assertInLoopThread();
    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);

    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EPollPoller::update(int operation, Channel* channel) {
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
              << " fd = " << fd << " event = { " << channel->eventsToString() << " }";
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_SYSERR << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
        } else {
            LOG_SYSFATAL << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
        }
    }
}
const char* EPollPoller::operationToString(int op) {
    switch (op) {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            assert(false && "ERROR op");
            return "Unknown Operation";
    }
}