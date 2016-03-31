//
// Created by root on 16-3-31.
//

#ifndef SERVER_LOGSTREAM_H_H
#define SERVER_LOGSTREAM_H_H

#include "StringPiece.h"
#include "Types.h"

#include <cassert>
#include <cstring> // memcpy

#ifndef  salangane_STD_STRING
#include <string>
#endif

#include <boost/noncopyable.hpp>

namespace salangane {
    namespace detail {
        const int kSmallBuffer = 4000;
        const int kLargeBuffer = 4000*1000;
        
        template <int SIZE>
        class FixedBuffer : boost::noncopyable {
        private:
            char data_[SIZE];
            char *cur_;
            void (*cookie_) ();
            
            const char *end() const {return data_ + sizeof(data_);}
            // must be outline function for cookies;
            static void cookieStart();
            static void cookieEnd();
        };
    }
}

#endif //SERVER_LOGSTREAM_H_H
