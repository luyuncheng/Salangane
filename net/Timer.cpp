//
// Created by root on 16-6-5.
//

#include "../net/Timer.h"

using namespace salangane;
using namespace salangane::net;

AtomicInt64 Timer::s_numCreated_;

void Timer::restart(Timestamp now) {
    if (repeat_) {
        expiration_ = addTime(now, interval_);
    } else {
        expiration_ = Timestamp::invalid();
    }
}