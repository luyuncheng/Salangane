//
// Created by root on 16-4-5.
//

#ifndef SALANGANE_CONDITION_H
#define SALANGANE_CONDITION_H

#include "Mutex.h"
#include <boost/noncopyable.hpp>
#include <pthread.h>

namespace salangane {
    class Condition : boost::noncopyable {
    public:
        explicit Condition(MutexLock &mutex) : mutex_(mutex) {
            // 被用来初始化一个条件变量。函数成功返回0；任何其他返回值都表示错误
            MCHECK(pthread_cond_init(&pcond_ , NULL));
        }
        ~Condition() {
            MCHECK(pthread_cond_destroy(&pcond_ ));
        }

        void wait() {
            // 条件变量是利用线程间共享的全局变量进行同步的一种机制，
            // 主要包括两个动作：一个线程等待"条件变量的条件成立"而挂起；
            // 另一个线程使"条件成立"（给出条件成立信号）。
            // 为了防止竞争，条件变量的使用总是和一个互斥锁结合在一起。
            MutexLock::UnassignGuard ug(mutex_);
            MCHECK(pthread_cond_wait(&pcond_, mutex_.getPthreadMutex()));
        }
        // returns true if time out, false otherwise.
        bool waitForSeconds(double seconds);

        void notify() {
            MCHECK(pthread_cond_signal(&pcond_));
        }
        void notifyAll() {
            MCHECK(pthread_cond_broadcast(&pcond_));
        }

    private:
        MutexLock &mutex_;
        pthread_cond_t pcond_;
    };
}


#endif //SALANGANE_CONDITION_H
