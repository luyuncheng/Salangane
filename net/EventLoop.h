//
// Created by root on 16-6-5.
//

#ifndef SALANGANE_EVENTLOOP_H
#define SALANGANE_EVENTLOOP_H

#include <vector>

#include <boost/any.hpp>
#include <boost/functional>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include "../base/Mutex.h"
#include "../base/CurrentThread.h"
#include "../base/Timestamp.h"
#include "../net/Callbacks.h"
#include "../net/TimerId.h"

//EventLoop 事件循环（反应器 Reactor），每个线程只能有一个 EventLoop 实
//体，它负责 IO 和定时器事件的分派。它用 eventfd(2) 来异步唤醒，这有别于传
// 统的用一对 pipe(2) 的办法。它用 TimerQueue 作为计时器管理，用 Poller 作
// 为 IO Multiplexing ( IO 多路复用)。
namespace salangane {
    namespace net {
        class Channel;
        class Poller;//轮询
        class TimerQueue;

        // Reactor at most one per thread
        // This is an interface class , so do not expose too much detais

        class EventLoop : boost::noncopyable {
        public:
            typedef boost::function<void()> Functor;

            EventLoop();
            ~EventLoop(); //force out-line dtor for scoped_ptr members
            ///
            /// loops forever
            ///
            /// Must be called in the same thread as creation of the object
            ///
            void loop();

            /// Quits loop
            ///
            /// This is not 100% threada safe, if you call through a raw pointer,
            /// better to call through shared_ptr<EventLoop> for 100% safety
            void quit();

            ///
            /// Time when poll returns, usually means data arrival.
            ///
            Timestamp pollReturnTIme() const {
                return pollerReturnTime_;
            }

            int64_t iteration() const {return iteration_;}

            /// Runs callback immediately in the loop thread.
            /// It weakes up the loop, and run the cb
            /// If in the same loop thread, cb is run within the function
            /// safe to call from other threads
            void runInLoop(const Functor &cb);

            ///queues callback in the loop thread.
            ///Runs after finish pooling.
            ///safe to call from other thread
            void queueInLoop(const Functor &cb);

#ifdef __GXX_EXPERMENTAL_CXX0X__
            void runInLoop(Functor &&cb);
            void queueInLoop(Functor &&cb);
#endif

        // timers

            //
            // Runs callback at 'time'
            // safe to call from other threads
            //
            TimerId runAt(const Timestamp &time, const TimerCallback &cb);

            //
            // Runs callback after @c delay seconds
            // safe to call from other threads
            //
            TimerId runAfter(double delay, const TimerCallback &cb);

            //
            // Runs callback ever @c interval seconds.
            // safe to call from other threads
            //
            TimerId runEvery(double interval, const TimerCallback& cb);

            //
            // Cancels the timer
            // safe to call from other threads
            //
            void cancel(TimerId timerId);

#ifdef __GXX_EXPERIMENTAL_CXX0X__
            TimerId runAt(const Timestamp& time, TimerCallback&& cb);
            TimerId runAfter(double delay, TimerCallback&& cb);
            TimerId runEvery(double interval, TimerCallback&& cb);
#endif

        // internal usage
            void wakeup();
            void updateChannel(Channel *channel);
            void removeChannel(Channel *channel);
            bool hasChannel(Channel *channel);

            //pid_t threadId() const {return threadId_;}
            void assertInLoopThread() {
                if(!isInLoopThread()) {
                    abortNotInLoopThread();
                }
            }
            bool isInLoopThread() const {return threadId_ == CurrentThread::tid();}
            // bool callingPendingFunctors() const {return callingPendingFunctors_;}
            bool eventHandling() const {return eventHandling_;}

            void setContext(const boost::any &context) {
                contxt_ = context;
            }

            const boost::any &getContext() const {
                return contxt_;
            }

            boost::any *getMutableContext() {
                return &contxt_;
            }
            static EventLoop* getEventLoopOfCurrentThread();

        private:
            void abortNotInLoopThread();
            void handleRead(); //waked up
            void doPendingFunctors();

            void printActiveChannels() const; //DEBUG

            typedef std::vector<Channel*> ChannelList;

            bool looping_; //atomic
            bool quit_; // atomic and shared between threads okay on x86 , i guess
            bool eventHandling_; //atomic;
            bool callingPendingFunctors_;//atoimic

            int64_t iteration_;
            const pid_t threadId_;
            Timestamp pollReturnTime_;
            // scoped_ptr can verify object can delete correct. it can not transfer the owner
            boost::scoped_ptr<Poller> poller_;
            boost::scoped_ptr<TimerQueue> timerQueue_;
            int wakeupFd_;
            // unlike in TimerQueue, which is an internal class
            // we do not expose channel to client
            boost::scoped_ptr<Channel> wakeupChannel_;
            boost::any contxt_;

            //scratch variables
            ChannelList activeChannels_;
            Channel *currentActiveChannel_;

            MutexLock mutex_;
            std::vector<Functor> pendingFunctors_; //@GuardedBy mutex

        };
    }
}

#endif //SALANGANE_EVENTLOOP_H
