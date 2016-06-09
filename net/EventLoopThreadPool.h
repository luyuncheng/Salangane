//
// Created by root on 16-6-9.
//

#ifndef SALANGANE_EVENTLOOPTHREADPOOL_H
#define SALANGANE_EVENTLOOPTHREADPOOL_H

#include "../base/Types.h"
#include <vector>
#include <functional>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace salangane{
    namespace net {
        class EventLoop;
        class EventLoopThread;

        class EventLoopThreadPool : boost :: noncopyable {
        public:
            typedef std::function<void(EventLoop*)> ThreadInitCallback;

            EventLoopThreadPool(EventLoop *baseLoop, const string &nameArg);
            ~EventLoopThreadPool();
            void setThreadNum(int numThreads) {numThreads_ = numThreads;}
            void start(const ThreadInitCallback &cb = ThreadInitCallback());

            // valid after calling start()
            // round-robin
            EventLoop *getNextLoop();

            // with the same hash code, it will always return the same EventLoop
            EventLoop* getLoopForHash(size_t hashCode);
            std::vector<EventLoop*> getAllLoops();

            bool started() const {
                return started_;
            }
            const string &name() const {
                return name_;
            }
        private:
            EventLoop *baseLoop_;
            string name_;
            bool started_;
            int numThreads_;
            int next_;
            boost::ptr_vector<EventLoopThread> threads_;
            std::vector<EventLoop *>loops_;
        };
    }
}

#endif //SALANGANE_EVENTLOOPTHREADPOOL_H
