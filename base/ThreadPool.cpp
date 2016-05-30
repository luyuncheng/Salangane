//
// Created by root on 16-5-30.
//

#include "ThreadPool.h"
#include "Exception.h"

#include <boost/bind.hpp>
#include <assert.h>
#include <stdio.h>

using namespace salangane;

ThreadPool::ThreadPool(const string &nameArg)
    : mutex_(),
      notEmpty_(mutex_),
      notFull_(mutex_),
      name_(nameArg),
      maxQueueSize_(0),
      running_(false)
{ }

ThreadPool::~ThreadPool() {
    if (running_) {
        stop();
    }
}

void ThreadPool::start(int numThreads) {
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(numThreads);
    for( int i = 0; i < numThreads; ++i) {
        char id[64];
        snprintf(id, sizeof(id), "%d", i+1);
        threads_.push_back(new salangane::Thread (
                // 将当前对象的跑在thread的函数和 当前this指针绑定？
                // (this) -> runInthread(name_+id)??
                boost::bind(&ThreadPool::runInThread, this), name_ + id)
        );
        threads_[i].start();
    }
    if (numThreads == 0 && threadInitCallback_) {
        threadInitCallback_();
    }
}

void ThreadPool::stop() {
    // add lock to mutex_
    MutexLockGuard lock(mutex_);
    running_ = false;
    notEmpty_.notifyAll();
    // for the first argument is for_each every threads bind to join?
    for_each(threads_.begin(), threads_.end(), boost::bind(&salangane::Thread::join, _1));
}

size_t ThreadPool::queueSize() const {
    MutexLockGuard lock(mutex_);
    return queue_.size();
}

void ThreadPool::run(const Task &task) {
    if (threads_.empty()) {
        task();
    } else {
        MutexLockGuard lock(mutex_);
        while(isFUll()) {
            // because is full so notFull condition must be pending
            notFull_.wait();
        }
        assert(!isFUll());

        queue_.push_back(task);
        // notify there is not empty condition, you can get this condition
        notEmpty_.notify();
    }
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
void ThreadPool::run(Task&& task)
{
    if (threads_.empty()) {
        task();
    }
    else
    {
        MutexLockGuard lock(mutex_);
        while (isFull())
        {
            notFull_.wait();
        }
        assert(!isFull());

        queue_.push_back(std::move(task));
        notEmpty_.notify();
    }
}
#endif

ThreadPool::Task ThreadPool::take() {
    MutexLockGuard lock(mutex_);
    // always use a while-loop, due to spurious wakeup
    while( queue_.empty() && running_) {
        // pending for not empty condition until add a task
        notEmpty_.wait();
    }
    Task task;
    if (!queue_.empty()) {
        task = queue_.front();
        queue_.pop_front();
        if (maxQueueSize_ > 0) {
            // it takes a task from queue, so when the notFull is wait(previous is full) then it can motivate the pending condition
            notFull_.notify();
        }
    }
    return task;
}

bool ThreadPool::isFull() const {
    // mutex_ must be locked
    mutex_.assertLocked();
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

