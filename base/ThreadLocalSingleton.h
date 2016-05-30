//
// Created by root on 16-5-30.
//

#ifndef SALANGANE_THREADLOCALSINGLETON_H
#define SALANGANE_THREADLOCALSINGLETON_H

#include <boost/noncopyable.hpp>
#include <assert.h>
#include <pthread.h>

namespace salangane {
    template <typename  T>
    class ThreadLocalSingleton : boost::noncopyable
    {
    public:
        static T& instance() {
            if(!t_value_) {
                t_value_ = new T();
                deleter_.set(t_value_);
            }
            return *t_value_;
        }
        static T*pointer() {
            return t_value_;
        }
    private:
        ThreadLocalSingleton();
        ~ThreadLocalSingleton();

        static void destructor(void *obj) {
            assert(obj == t_value_);
            typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
            T_must_be_complete_type dummy; (void) dummy;
            delete t_value_;
            t_value_ = 0;
        }
//        static void destructor (void *x) {
//            assert(x == t_value_);
//            T *obj = static_cast<T*>(x);
//            typedef char T_must_be_comlete_type[sizeof(T) == 0 ? -1 : 1];
//            T_must_be_comlete_type dummy;
//            // 防止编译器报警报, 告诉编译器这个变量没被用过
//            (void) dummy;
//            delete obj;
//        }

        class Deleter {
        public:
            Deleter() {
                pthread_key_create(&pkey_, &ThreadLocalSingleton::destructor);
            }
            ~Deleter() {
                pthread_key_t (pkey_);
            }

            void set(T *newObj) {
                assert(pthread_getspecific(pkey_) == NULL);
                pthread_setspecific(pkey_, newObj);
            }
            pthread_key_t  pkey_;
        };

        // __thread变量每一个线程有一份独立实体，各个线程的值互不干扰
        static __thread T* t_value_;
        static Deleter deleter_;
    };

    template <typename T>
    __thread T* ThreadLocalSingleton<T>::t_value_ = 0;

    template <typename T>
    typename ThreadLocalSingleton<T>::Deleter ThreadLocalSingleton<T>::deleter_;
}


#endif //SALANGANE_THREADLOCALSINGLETON_H
