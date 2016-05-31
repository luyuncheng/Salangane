//
// Created by root on 16-5-29.
//

#ifndef SALANGANE_THREADLOCAL_H
#define SALANGANE_THREADLOCAL_H

#include "Mutex.h"
#include <boost/noncopyable.hpp>
#include <pthread.h>

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
            // //同一线程内的各个函数间共享数据。
            T* perThreadValue = static_cast<T*> (pthread_getspecific(pkey_));
            if (!perThreadValue) {
                T *newObj = new T();
                MCHECK(pthread_setspecific(pkey_, newObj));
                perThreadValue = newObj;
            }
            return *perThreadValue;
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

#endif //SALANGANE_THREADLOCAL_H