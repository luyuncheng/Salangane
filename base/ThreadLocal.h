//
// Created by root on 16-5-29.
//

#ifndef SALANGANE_THREADLOCAL_H
#define SALANGANE_THREADLOCAL_H

#include "Mutex.h"
#include <boost/noncopyable.hpp>
#include <pthread.h>
#endif //SALANGANE_THREADLOCAL_H

namespace salangane {
    template <typename T>
    class ThreadLocal : boost::noncopyable {
    public:
        ThreadLocal() {
            // system create a ptr pointer to the key , the second parameter is destructor
            MCHECK(pthread_key_create(&pkey_, &ThreadLocal::destructor));
        }
        ~ThreadLocal() {
            MCHECK(pthread_key_delete(pkey_));
        }

        T &value () {

        }
    private:
        static void destructor (void *x) {
            T *obj = static_cast<T*>(x);
            typedef char T_must_be_comlete_type[sizeof(T) == 0 ? -1 : 1];
            T_must_be_comlete_type dummy;
            // 防止编译器报警报, 告诉编译器这个变量没被用过
            (void) dummy;
            delete obj;
        }

        pthread_key_t pkey_;
    };

}