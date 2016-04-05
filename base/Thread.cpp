//
// Created by root on 16-3-31.
//

#include "Thread.h"
#include "CurrentThread.h"
#include "Exception.h"
#include "Logging.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <errno.h>
#include <cstdio>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <linux/unistd.h>

namespace salagane {
    namespace CurrentThread {
        __thread int t_cachedTid = 0;
        __thread char t_tidString[32];
        __thread int t_tidStringLength = 6;
        __thread const char* t_threadName = "unknown";
        const bool sameType = boost::is_same<int, pid_t >::value;
        BOOST_STATIC_ASSERT(sameType);
    };

    namespace detail {

        pid_t gettid() {
            return static_cast<pid_t>(::syscall(SYS_gettid));
        }

        void afterFork() {
            salangane::CurrentThread::t_cachedTid = 0;
            salangane::CurrentThread::t_threadName = "main";
            salangane::CurrentThread::tid();
            // no need to call pthread_atfork(NULL, NULL, &afterFork);
        }

        class ThreadNameInitializer {
        public:
            ThreadNameInitializer() {
                salangane::CurrentThread::t_threadName = "main";
                salangane::CurrentThread::tid();
                // 该函数的作用就是往进程中注册三个函数，以便在不同的阶段调用
                // 新进程产生之前被调用, 新进程产生之后在父进程被调用, 新进程产生之后，在子进程被调用
                pthread_atfork(NULL, NULL, &afterFork);
            }
        };
        ThreadNameInitializer init;

        struct ThreadData {
            typedef salangane::Thread::ThreadFunc ThreadFunc;
            ThreadFunc func_;
            salangane::string name_;
//            boost::weak_ptr只对boost::shared_ptr进行引用，而不改变其引用计数。
            boost::weak_ptr<pid_t> wkTid_;

            ThreadData(const ThreadFunc &func,
                       const salangane::string &name,
                       const boost::shared_ptr<pid_t> &tid) :
                       func_(func), name_(name), wkTid_(tid) { }

            void runInThread() {
                pid_t tid = salangane::CurrentThread::tid();
                boost::shared_ptr<pid_t> ptid = wkTid_.lock();
                if(ptid) {
                    *ptid = tid;
                    ptid.reset();
                }

                salangane::CurrentThread::t_threadName = name_.empty() ? "salanganThread" : name_.c_str();
                //用prctl给线程命名
                ::prctl(PR_SET_NAME, salangane::CurrentThread::t_threadName);

                try {
                    func_();
                    salangane::CurrentThread::t_threadName = "finished";
                } catch (const salangane::Exception &ex) {
                    salangane::CurrentThread::t_threadName = "crashed";
                    fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
                    fprintf(stderr, "reason: %s\n", ex.what());
                    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
                    abort();
                }catch (const std::exception &ex) {
                    salangane::CurrentThread::t_threadName = "crashed";
                    fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
                    fprintf(stderr, "reason: %s\n",ex.what());
                    abort();
                } catch (...) {
                    salangane::CurrentThread::t_threadName = "crashed";
                    fprintf(stderr, "Unknown exception caught in Thread %s\n", name_.c_str());
                    throw;
                }
            }
        };

        void *startThread(void *obj) {
            ThreadData *data = static_cast<ThreadData *>(obj);
            data->runInThread();
            delete data;
            return NULL;
        }
    }
}
using namespace salangane;
void CurrentThread::cacheTid() {
    if (t_cachedTid == 0) {
        t_cachedTid = detail::gettid();
        t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
    }
}
bool CurrentThread::isMainThread() {
    return tid() == ::getpid();
}
bool CurrentThread::sleepUsec(int64_t usec) {
    struct timespec ts = {0, 0};
    ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
    ::nanosleep(&ts,NULL);
}

AtomicInt32 Thread::numCreated_;

Thread::Thread(const ThreadFunc& func, const string& n)
        : started_(false),
          joined_(false),
          pthreadId_(0),
          tid_(new pid_t(0)),
          func_(func),
          name_(n)
{
    setDefaultName();
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
Thread::Thread(ThreadFunc&& func, const string& n)
        : started_(false),
          joined_(false),
          pthreadId_(0),
          tid_(new pid_t(0)),
          func_(std::move(func)),
          name_(n)
{
    setDefaultName();
}

#endif

Thread::~Thread() {
    if(started_ && !joined_) {
        pthread_detach(pthreadId_);
    }
}

void Thread::setDefaultName() {
    int num = numCreated_.incrementAndGet();
    if(name_.empty()) {
        char buf[32];
        snprintf(buf,sizeof(buf),"Thread%d",num);
        name_ = buf;
    }
}

void Thread::start() {
    assert(!started_);
    started_ = true;
    detail::ThreadData *data = new detail::ThreadData(func_, name_, tid_);
    if(pthread_create(&pthreadId_, NULL, &detail::startThread, data)) {
        started_ = false;
        delete data;
        LOG_SYSFATAL << "Failed in pthread_create";
    }
}
int Thread::join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}