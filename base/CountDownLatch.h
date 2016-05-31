//
// Created by root on 16-5-30.
//
//“倒计时门闩”同步
#ifndef SALANGANE_COUNTDOWNLATCH_H
#define SALANGANE_COUNTDOWNLATCH_H

#include "Condition.h"
#include "Mutex.h"

#include <boost/noncopyable.hpp>

namespace salangane {
    class CountDownLatch : boost :: noncopyable {
    public:
        explicit CountDownLatch (int count);

        void wait();

        void countDown();

        int getCount() const;

    private:
        mutable MutexLock mutex_;
        Condition condition_;
        int count_;
    };
}
#endif //SALANGANE_COUNTDOWNLATCH_H
