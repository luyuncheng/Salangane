//
// Created by root on 16-5-31.
//

#ifndef SALANGANE_BOUNDEDBLOCKINGQUEUE_H
#define SALANGANE_BOUNDEDBLOCKINGQUEUE_H

#include "Condition.h"
#include "Mutex.h"

#include <boost/circular_buffer.hpp>
#include <boost/noncopyable.hpp>
#include <assert.h>

namespace salangane {
    template <typename T>
    class BoundedBlockingQueue : boost::noncopyable {
    public:
        explicit BoundedBlockingQueue(int maxSize)
            : mutex_(),
              notEmpty_ (mutex_),
              notFull_ (mutex_),
              queue_ (maxSize)
        { }

        void put(const T&x) {
            MutexLockGuard lock(mutex_);
            while (queue_.full()) {
                notFull_.wait();
            }
            assert(!queue_.full());
            queue_.push_back(x);
            notEmpty_.notify();
        }

        T take() {
            MutexLockGuard lock(mutex_);
            while(queue_.empty()) {
                notEmpty_.wait();
            }
            assert(!queue_.empty());
            T front(queue_.front());
            queue_.pop_front();
            notFull_.notify();
            return front();
        }

        bool empty() const {
            MutexLockGuard lock(mutex_);
            return queue_.empty();
        }

        bool full() const {
            MutexLockGuard lock(mutex_);
            return queue_.full();
        }

        size_t size() const {
            MutexLockGuard lock(mutex_);
            return queue_.size();
        }

        size_t capacity() const {
            MutexLockGuard lock(mutex_);
            return queue_.capacity();
        }
    private:
        mutable MutexLock mutex_;
        Condition notEmpty_;
        Condition notFull_;
        //circular_buffer is limited as list when over the maxsize than override the first data
        boost::circular_buffer<T> queue_;
    };
}

#endif //SALANGANE_BOUNDEDBLOCKINGQUEUE_H
