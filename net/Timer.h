//
// Created by root on 16-6-5.
//

#ifndef SALANGANE_TIMER_H
#define SALANGANE_TIMER_H

#include <boost/noncopyable.hpp>

#include "../base/Atomic.h"
#include "../base/Timestamp.h"
#include "../net/Callbacks.h"

namespace salangane {
    namespace net {
        // Internal class for tmer event
        class Timer : boost::noncopyable {
        public:
            Timer(const TimerCallback& cb, Timestamp when, double interval)
                    : callback_(cb),
                      expiration_(when),
                      interval_(interval),
                      repeat_(interval > 0.0),
                      sequence_(s_numCreated_.incrementAndGet())
            { }
            void run() const {
                callback_();
            }

            Timestamp expiration() const  { return expiration_; }
            bool repeat() const { return repeat_; }
            int64_t sequence() const { return sequence_; }

            void restart(Timestamp now);

            static int64_t numCreated() { return s_numCreated_.get(); }
        private:
            const TimerCallback callback_;
            // 截止日期
            Timestamp expiration_;
            const double interval_;
            const bool repeat_;
            const int64_t sequence_;

            static AtomicInt64 s_numCreated_;
        };
    }
}

#endif //SALANGANE_TIMER_H
