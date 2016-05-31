//
// Created by root on 16-5-31.
//

#ifndef SALANGANE_BLOCKINGQUEUE_H
#define SALANGANE_BLOCKINGQUEUE_H


#include "Condition.h"
#include "Mutex.h"

#include <boost/noncopyable.hpp>
#include <deque>
#include <assert.h>

namespace salangane {
    template <typename T>
    class BlockingQueue : boost::noncopyable {
    public:
        BlockingQueue()
                : mutex_(),
                  nonEmpty_(mutex_),
                  queue_()
        {}

        void put (const T& x) {
            MutexLockGuard lock(mutex_);
            queue_.push_back(x);

            // wait morphing saves us
            nonEmpty_.notify();
        }

#ifdef __GXX_EXPERIMENTAL_CXX0x_
      void put (T &&x) {
        MutexLockGuard lock(mutex_);
        queue_.push_back(std::move(x));
        notEmpty_.notify();
      }
#endif

        T take() {
            MutexLockGuard lock(mutex_);
            while(queue_.empty()) {
                nonEmpty_.wait();
            }
            assert(!queue_.empty());

#ifdef __GXx_EXPERIMENTAL_CXX0X_
            T front(std::move(queue_.front()));
#else
            T front(queue_.front());
#endif
            queue_.pop_front();
            return front;
        }

        size_t size() const {
            MutexLock lock(mutex_);
            return queue_.size();
        }
    private:
        mutable MutexLock mutex_;
        Condition nonEmpty_;
        std::deque<T> queue_;
    };
}

#endif //SALANGANE_BLOCKINGQUEUE_H
