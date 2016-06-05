//
// Created by root on 16-6-4.
//

#include "../ThreadPool.h"
#include "../CountDownLatch.h"
#include "../CurrentThread.h"
#include "../Logging.h"

#include <boost/bind.hpp>
#include <stdio.h>
#include <functional>

void print() {
    printf("tid=%d\n", salangane::CurrentThread::tid());
}

void printString(const std::string& str)
{
    LOG_INFO << str;
    usleep(100*1000);
}

void test(int maxSize) {
    LOG_WARN << "Test ThreadPool with max queue size = " << maxSize;
    salangane::ThreadPool pool("MainThreadPool");
    pool.setMaxQueueSize(maxSize);
    pool.start(5);

    LOG_WARN << "Adding";

    pool.run(print);
    pool.run(print);
    for (int i = 0; i < 100; ++i) {

        char buf[32];
        snprintf(buf, sizeof buf, "task %d", i);
        pool.run(boost::bind(printString, std::string(buf)));
    }
    LOG_WARN << "Done";

    salangane::CountDownLatch latch(1);
    // wrong
    //auto lt = std::bind(&(salangane::CountDownLatch::countDown), std::ref(latch));

    //black also wrong
    //auto lt = std::bind(decltype(&salangane::CountDownLatch::countDown)(&(salangane::CountDownLatch::countDown)),latch);

    auto lt = std::bind(&salangane::CountDownLatch::countDown, &latch);
    //auto l1 = &salangane::CountDownLatch::countDown;
    //auto l2 = &(salangane::CountDownLatch::countDown);
    pool.run(lt);
    latch.wait();
    pool.stop();
}

int Thread_pool_test()
{
  test(0);
  test(1);
  test(5);
  test(10);
  test(50);
}
