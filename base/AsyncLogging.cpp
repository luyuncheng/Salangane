//
// Created by root on 16-6-1.
//

#include "AsyncLogging.h"
#include "LogFile.h"
#include "Timestamp.h"

#include <stdio.h>

using namespace salangane;

AsyncLogging::AsyncLogging(const string &basename,
                           size_t rollSize,
                           int flushInterval)
    : flushInterval_ (flushInterval_),
      running_ (false),
      basename_ (basename),
      rollSize_ (rollSize),
      thread_ (boost::bind(&AsyncLogging::threadFunc, this),"Logging"),
      latch_(1),
      mutex_(),
      cond_(mutex_),
      currentBuffer_ (new Buffer),
      nextBuffer_ (new Buffer),
      buffers_()
{
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

void AsyncLogging::threadFunc() {
    assert(running_ == true);
    latch_.countDown();
    LogFile output(basename_, rollSize_, false);

    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);
    while (running_) {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());
        salangane::MutexLockGuard lock(mutex_);
        if (buffers_.empty()) {
            cond_.waitForSeconds(flushInterval_);
        }
        buffers_.push_back(currentBuffer_.release());
        currentBuffer_ = (BufferPtr) boost::ptr_container::move(newBuffer1);
        buffersToWrite.swap(buffers_);
        if (!nextBuffer_) {
            nextBuffer_ = (BufferPtr) boost::ptr_container::move(newBuffer2);
        }


        assert(!buffersToWrite.empty());
        if (buffersToWrite.size() > 25) {
            char buf[256];
            snprintf(buf, sizeof(buf), "Dropped log messages at %s, %zd larger buffers\n",
                     Timestamp::now().toFormattedString().c_str(),
                     buffersToWrite.size() - 2);
            fputs(buf, stderr);
            output.append(buf, static_cast<int>(strlen(buf)));
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
        }

        for (size_t i = 0; i < buffersToWrite.size(); ++i) {
            output.append(buffersToWrite[i].data(), buffersToWrite[i].length());
        }

        if (buffersToWrite.size() > 2) {
            buffersToWrite.resize(2);
        }

        if (!newBuffer1) {
            assert(!buffersToWrite.empty());
            newBuffer1 = buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2) {
            assert(!buffersToWrite.empty());
            newBuffer2 = buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        output.flush();
    }
    output.flush();
}
