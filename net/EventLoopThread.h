//
// Created by root on 16-6-9.
//

#ifndef SALANGANE_EVENTLOOPTHREAD_H
#define SALANGANE_EVENTLOOPTHREAD_H

#include "../base/Condition.h"
#include "../base/Mutex.h"
#include "../base/Thread.h"

#include <boost/noncopyable.hpp>
#include <functional>
namespace salangane {
    namespace net {
        class EventLoop;

        class EventLoopThread : boost::noncopyable {
        public:
            typedef std::function<void(EventLoop *)> ThreadInitCallback;

            EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                            const string& name = string());
            ~EventLoopThread();
            EventLoop *startLoop();

        private:
            void threadFunc();

            EventLoop *loop_;
            bool exiting_;
            Thread thread_;
            MutexLock mutex_;
            Condition cond_;
            ThreadInitCallback callback_;
        };
    }
}

#endif //SALANGANE_EVENTLOOPTHREAD_H
