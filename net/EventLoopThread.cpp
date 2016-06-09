//
// Created by root on 16-6-9.
//

#include "../net/EventLoopThread.h"
#include "../net/EventLoop.h"
#include <boost/bind.hpp>

using namespace salangane;
using namespace salangane::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb,
                                 const string &name)
    : loop_(NULL),
      exiting_(false),
      thread_(boost::bind(&EventLoopThread::threadFunc,this),name),
      mutex_(),
      cond_(mutex_),
      callback_(cb)
{ }

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    // not 100% race-free, eg. threadFunc could be running callback_.
    if(loop_ != NULL) {
        // still a tiny chance to call destructed object, if threadFUnc exits just now
        // but when EventLoop Thread destructs, usually programming is exiting anyway
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop() {
    assert(!thread_.started());
    thread_.start();

    MutexLockGuard lock(mutex_);
    while(loop_ == NULL) {
        cond_.wait();
    }
    return loop_;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    if(callback_) {
        callback_(&loop);
    }

    MutexLockGuard lock(mutex_);
    loop_ = &loop;
    cond_.notify();

    loop.loop();
    loop_=NULL;
}