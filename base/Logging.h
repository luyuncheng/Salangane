//
// Created by root on 16-3-31.
//
//简单的日志，可搭配 AsyncLogging 使用
#ifndef SERVER_LOGGING_H
#define SERVER_LOGGING_H

//#include "LogStream.h"
//#include "TimeStamp.h"

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
            template <int N>
        };
    };
}
#endif //SERVER_LOGGING_H
