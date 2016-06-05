//
// Created by root on 16-6-5.
//

#ifndef SALANGANE_TIMERID_H
#define SALANGANE_TIMERID_H

#include <../base/copyable.h>
#include <zconf.h>
#include "../base/copyable.h"

namespace salangane {
    namespace net {
        class Timer;
        class TimerId : public salangane::copyable
        {
        public:
            TimerId() : timer_(NULL), sequence_(0) {}
            TimerId(Timer *timer, int64_t seq) : timer_(timer), sequence_(seq) {}

            //default copy-ctor, dtor and assignment are okay
            friend class TimerQueue;
        private:
            Timer *timer_;
            int64_t sequence_;
        };
    }
}
#endif //SALANGANE_TIMERID_H
