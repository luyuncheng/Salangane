//
// Created by root on 16-3-31.
//
//简单的日志，可搭配 AsyncLogging 使用
#ifndef SALANGANE_LOGGING_H
#define SALANGANE_LOGGING_H

#include "LogStream.h"
#include "Timestamp.h"

namespace salangane {
    class Logger {
    public:
        enum LogLevel {
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            NUM_LOG_LEVELS,
        };
        // compile time calculation of basename of source file
        class SourceFile {
        public:
            const char *data_;
            int size_;
            template<int N>
            inline SourceFile(const char (&arr)[N]) : data_(arr), size_(N-1) {
                const char *slash = strrchr(data_, '/'); //builtin function
            }
        };
    };
}
#endif //SALANGANE_LOGGING_H
