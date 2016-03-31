//
// Created by root on 16-3-28.
//

#ifndef SERVER_UTIL_H
#define SERVER_UTIL_H

#include <string>
#include <vector>
#include <cstring>
#include <functional>
#include <utility>
#include <cstdlib>
#endif //SERVER_UTIL_H

namespace handy {
    struct util {
        static std::string format(const char *fmt,...);
        static int64_t timeMicro();
        static int64_t timeMilli() {return timeMicro()/1000;}
        static int64_t steadyMicro();
        static int64_t steadyMilli() {return steadyMicro()/1000;}
        static std::string readableTime(time_t t);
//       e为下一个地址
        static int64_t atoi(const char *b, const char *e,int base = 10) {
            return strtol(b,(char **)&e,base);
        }
        static int64_t atoi2(const char *b,const char *e) {
            char **ne = (char **)&e;
            int64_t v = strtol(b,ne,10);
            return ne == (char**)&e ? v:-1;
        }
        static int64_t atoi(const char *b) {return atoi(b,b+strlen(b));}
        static int addFdFlag(int fd,int flag);
    };
    struct ExitCaller {
     private:
        std::function<void()> functor_;
        ~ExitCaller() { functor_(); }
        ExitCaller(std::function<void()>&& functor): functor_(std::move(functor)) {}
   };
}

