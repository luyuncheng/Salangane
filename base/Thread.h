//
// Created by root on 16-3-29.
//

#ifndef SALANGANE_THREAD_H
#define SALANGANE_THREAD_H

#include "Atomic.h"
#include "Types.h"

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <pthread.h>

namespace salangane {

    class Thread : boost::noncopyable {
    private:
        void setDefaultName();

        bool started_;
        bool joined_;
        pthread_t pthreadId_;
        boost::shared_ptr<pid_t> tid_;
        ThreadFunc func_;
        string name_;

        static AtomicInt32 numCreated_;

    public:
        typedef boost::function<void ()> ThreadFunc;
//        声明为explicit的构造函数不能在隐式转换中使用
        explicit Thread(const ThreadFunc&, const string &name = string());

#ifdef __GXX_EXPERIMENTAL_CXX0X__
        explicit Thread(ThreadFunc&&, const string& name = string());
#endif
        ~Thread();

        void start();

        //return pthread_join()
        int join();

        bool started() const { return started_; }

//        pthread_t pthreadId() const {return pthreadId_;}
        pid_t tid() const { return *tid_; }
        const string &name() const { return name_; }
        static int numCreated() { return numCreated_.get(); }
    };
}

#endif //SALANGANE_THREAD_H

