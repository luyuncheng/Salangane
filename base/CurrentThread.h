//
// Created by root on 16-3-31.
//

#ifndef SALANGANE_CURRENTTHREAD_H
#define SALANGANE_CURRENTTHREAD_H

#include <stdint.h>

namespace salangane {
    namespace CurrentThread {
        // internal
//        __thread是GCC内置的线程局部存储设施,存取效率可以和全局变量相比,__thread值线程间互不干扰
        extern __thread int t_cachedTid;
        extern __thread char t_tidString[32];
        extern __thread int t_tidStringLength;
        extern __thread const char* t_threadName;
        void cacheTid();

        inline int tid() {
//            表示为假的可能性比较大 预先读取值
            if (__builtin_expect(t_cachedTid == 0, 0)) {
                cacheTid();
            }
            return t_cachedTid;
        }

        inline const char* tidString() { // for logging
            return t_tidString;
        }

        inline int tidStringLength() { //for logging
            return t_tidStringLength;
        }

        inline const char* name() {
            return t_threadName;
        }

        bool isMainThread();

        void sleepUsec(int64_t usec);
    }
}

#endif //SALANGANE_CURRENTTHREAD_H
