//
// Created by root on 16-3-31.
//

//带 stack trace 的异常基类
#ifndef SALANGANE_EXCEPTION_H
#define SALANGANE_EXCEPTION_H

#include "Types.h"
#include <exception>

namespace salangane {
    class Exception : public std::exception {


    public:
        explicit Exception (const char *what);
        explicit Exception (const string &what);
        virtual ~Exception() throw();
        virtual const char *what() const throw();
        const char *stackTrace() const throw();

    private:
       void fillStackTrace();

        string message_;
        string stack_;


    };
}
#endif //SALANGANE_EXCEPTION_H
