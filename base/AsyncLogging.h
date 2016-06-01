//
// Created by root on 16-6-1.
//

#ifndef SALANGANE_ASYNCLOGGING_H
#define SALANGANE_ASYNCLOGGING_H

#include "BlockingQueue.h"
#include "BoundedBlockingQueue.h"
#include "CountDownLatch.h"
#include "Mutex.h"
#include "Thread.h"
#include "LogStream.h"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace salangane {
    class AsyncLogging : boost :: noncopyable {
    public:
        AsyncLogging (const string& basename,
                      size_t rollSize,
                      int flushInterval = 3);
        ~AsyncLogging() {
            if(running_) {
                stop();
            }
        }

        void append(const char *logline, int len);

        void start() {
            running_ = true;
            thread_.start();
            latch_.wait();
        }

        void stop() {
            running_ = false;
            cond_.notify();
            thread_.join();
        }
    private:

        // declare but not define, prevent compiler-synthesized functions
        AsyncLogging(const AsyncLogging &); // ptr_container
        void operator=(const AsyncLogging &); //ptr_container

        void threadFunc();

        typedef salangane::detail::FixedBuffer<salangane::detail::kLargeBuffer> Buffer;
        typedef boost::ptr_vector<Buffer> BufferVector;
        typedef BufferVector::auto_type BufferPtr;

        const int flushInterval_;
        bool running_;
        string basename_;
        size_t rollSize_;
        salangane::Thread thread_;
        salangane::CountDownLatch latch_;
        salangane::MutexLock mutex_;
        salangane::Condition cond_;
        BufferPtr currentBuffer_;
        BufferPtr nextBuffer_;
        BufferVector buffers_;

    };
}

#endif //SALANGANE_ASYNCLOGGING_H
