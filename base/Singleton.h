//
// Created by root on 16-6-1.
//

#ifndef SALANGANE_SINGLETON_H
#define SALANGANE_SINGLETON_H

#include <boost/noncopyable.hpp>
#include <assert.h>
#include <stdlib.h> //atexit
#include <pthread.h>

namespace salangane {
    namespace detail {

        // this does not detect inherited member functions!
        // http://stackoverflow.com/questions/1966362/sfinae-to-check-for-inherited-member-functions
        template <typename  T>
        struct has_no_destroy {
            // for decltype(e) before it returns it runs
            // if expression is local variate or namespace action variate or siatic variate or function argument it returns type of declare
            // if e is lvalue (can reachable by address) it reurns T& which T is typeof e
            // if e is xvalue return T&&
            // if e is prvalue return T
            // eg:
            // const int && foo(); const int bar(); int i; struct A {double x;};
            // const A* a = new A();
            // decltype(foo()) x1; // type is const int &&
            // decltype(bar()) x2; // type is int
            // decltype(i) x3; // type is int;
            // decltype(a->x) x4 // type is double
            // decltype( (a->x) ) x5 // type is const double &
            template <typename C> static char test(typeof(&C::no_destry)); // or decltype in C++11
            template <typename C> static int32_t test(...);
            const static bool value = sizeof(test<T>(0)) == 1;

        };
    }

    template <typename T>
    class Singleton : boost::noncopyable {
    public:
        static T& instance() {
            // pthread_once use ponce_(pthread_once_t) to guarantee init function only run once
            pthread_once(&ponce_, &Singleton::init);
            assert(value_ != NULL);
            return *value_;
        }

    private:
        Singleton();
        ~Singleton();

        static void init() {
            value_ = new T();
            if(!detail::has_no_destroy<T>::value) {
                ::atexit(destroy);
            }
        }
        static  void destroy() {
            typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
            T_must_be_complete_type dummy; (void) dummy;

            delete value_;
            value_ = NULL;
        }

        static pthread_once_t ponce_;
        static T* value_;

    };
}

#endif //SALANGANE_SINGLETON_H
