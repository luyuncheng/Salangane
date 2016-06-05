//
// Created by root on 16-6-5.
//

#include "../base/Logging.h"
#include "../net/Channel.h"
#include "../net/EventLoop.h"

#include <sstream>
#include <poll.h>

using namespace salangane;
using namespace salangane::net;
const int Channel::kNoneEvent = 0;
//poll()的参数pollfd.events可以设置若干种监视的状态:
//POLLIN:                有普通数据或者优先数据可读
//POLLRDNORM:    有普通数据可读
//POLLRDBAND:    有优先数据可读
//POLLPRI:              有紧急数据可读
//POLLOUT:            有普通数据可写
//POLLWRNORM:   有普通数据可写
//POLLWRBAND:    有紧急数据可写
//POLLERR:            有错误发生
//POLLHUP:            有描述符挂起事件发生
//POLLNVAL:          描述符非法

const int Channel::kReadEvent = POLLIN | POLLPRI ;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd__)
        : loop_(loop),
          fd_(fd__),
          events_(0),
          revents_(0),
          index_(-1),
          logHup_(true),
          tied_(false),
          eventHandling_(false),
          addedToLoop_(false)
{ }

Channel::~Channel() {
    assert(!eventHandling_);
    assert(!addedToLoop_);
    if (loop_->isInLoopThread()) {
        assert(!loop_->hasChannel(this));
    }
}

void Channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    tied_ = true;
}

void Channel::update() {
    addedToLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::remove() {
    assert(isNoneEvent());
    addedToLoop_ = false;
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime) {
    std::shared_ptr<void> guard;
    if (tied_) {
        guard = tie_.lock();
        if (guard) {
            handleEventWithGuard(receiveTime);
        }
    } else {
        Channel::handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    eventHandling_ = true;
    LOG_TRACE << reventsToString();
    // 有描述符挂起事件发生 且 并非 有普通数据或者优先数据可读
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        if (logHup_) {
            LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
        }
        if (closeCallback_) closeCallback_();
    }
    //描述符非法
    if (revents_ & POLLNVAL) {
        LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
    }
    //描述符非法 ||  有错误发生
    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallback_)
            errorCallback_();
    }
    //有普通数据或者优先数据可读 || 有紧急数据可读 || 有描述符挂起事件发生
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback_)
            readCallback_(receiveTime);
    }
    //有普通数据可写
    if (revents_ & POLLOUT) {
        if (writeCallback_)
            writeCallback_();
    }
    eventHandling_ = false;
}

string Channel::reventsToString() const {
    return eventsToString(fd_, revents_);
}

string Channel::eventsToString() const {
    return eventsToString(fd_, events_);
}

string Channel::eventsToString(int fd, int ev) {
    std::ostringstream oss;
    oss << fd << ": ";
    if (ev & POLLIN)    oss << "IN ";
    if (ev & POLLPRI)   oss << "PRI ";
    if (ev & POLLOUT)   oss << "OUT ";
    if (ev & POLLHUP)   oss << "HUP ";
    if (ev & POLLRDHUP) oss << "RDHUP ";
    if (ev & POLLERR)   oss << "ERR ";
    if (ev & POLLNVAL)  oss << "NVAL ";

    return oss.str().c_str();
}