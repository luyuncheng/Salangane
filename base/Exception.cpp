//
// Created by root on 16-3-31.
//

#include "Exception.h"

#include <execinfo.h>
#include <stdlib.h>

using namespace salangane;
void Exception::fillStackTrace() {
    const int len = 200;
    void *buffer[len];
    int nptrs = :: backtrace(buffer,len);
    char **strings = ::backtrace_symbols(buffer, nptrs);
    if(string) {
        for(int i = 0; i<len; i++) {
            stack_.append(strings[i]);
            stack_.push_back('\n');
        }
        free(strings);
    }
}
Exception::Exception(const char* msg) : message_(msg) {
    fillStackTrace();
}
Exception::Exception(const string& msg) : message_(msg) {
    fillStackTrace();
}
Exception::~Exception() throw(){

}
const char* Exception::stackTrace() const throw(){
    return stack_.c_str();
}
const char* Exception::what() const throw(){
    return message_.c_str();
}
