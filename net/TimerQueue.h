//
// Created by root on 16-6-5.
//

#ifndef SALANGANE_TIMERQUEUE_H
#define SALANGANE_TIMERQUEUE_H

#include <set>
#include <vector>

#include <boost/noncopyable.hpp>
#include "../base/Mutex.h"
#include "../base/Timestamp.h"
#include "../net/Callbacks.h"
#include "../net/Channel.h"
#include "../net/TimerId.h"
//timerfd是Linux为用户程序提供的一个定时器接口。这个接口基于文件描述符，通过文件描述符的可读事件进行超时通知，
//TimerQueue 用 timerfd 实现定时，这有别于传统的设置 poll/epoll_wait 的等
//待时长的办法。TimerQueue 用 std::map 来管理 Timer，常用操作的复杂度是
//O(ln N)，N 为定时器数目。它是 EventLoop 的成员，生命期由后者控制。

namespace salangane {
    namespace net {
        class EventLoop;
        class Timer;
        class TimerId;
        ///
        /// A best efforts timer queue.
        /// No guarantee that the callback will be on time.

        class TimerQueue : boost::noncopyable {
        public:
            TimerQueue(EventLoop *loop);
            ~TimerQueue();

        ///
        /// Schedules the callback to be run at given time,
        /// repeats if @c interval > 0.0.
        ///
        /// Must be thread safe. Usually be called from other threads.
            TimerId addTimer(const TimerCallback& cb,
                             Timestamp when,
                             double interval);

            void cancel(TimerId timerId);

        private:

            // FIXME: use unique_ptr<Timer> instead of raw pointers.
            typedef std::pair<Timestamp, Timer*> Entry;
            typedef std::set<Entry> TimerList;
            typedef std::pair<Timer*, int64_t> ActiveTimer;
            typedef std::set<ActiveTimer> ActiveTimerSet;

            void addTimerInLoop(Timer* timer);
            void cancelInLoop(TimerId timerId);
            // called when timerfd alarms
            void handleRead();
            // move out all expired timers
            std::vector<Entry> getExpired(Timestamp now);
            void reset(const std::vector<Entry>& expired, Timestamp now);

            bool insert(Timer* timer);

            EventLoop* loop_;
            const int timerfd_;
            Channel timerfdChannel_;
            // Timer list sorted by expiration
            TimerList timers_;

            // for cancel()
            ActiveTimerSet activeTimers_;
            bool callingExpiredTimers_; /* atomic */
            ActiveTimerSet cancelingTimers_;
        };
    }
}

#endif //SALANGANE_TIMERQUEUE_H
