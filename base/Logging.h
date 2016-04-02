//
// Created by root on 16-3-31.
//
//简单的日志，可搭配 AsyncLogging 使用
#ifndef SALANGANE_LOGGING_H
#define SALANGANE_LOGGING_H

#include "LogStream.h"
#include "Timestamp.h"

namespace salangane {
    class TimeZone;
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
                if(slash) {
                    data_  = slash + 1;
                    size_ -= static_cast<int>(data_ - arr);
                }
            }

            explicit SourceFile (const char * filename) : data_(filename) {
                const char *slash = strrchr(filename, '/');
                if (slash) {
                    data_ = slash + 1;
                }
                size_ = static_cast<int>(strlen(data_));
            }

        };

        Logger(SourceFile file, int line);
        Logger(SourceFile file, int line, LogLevel level);
        Logger(SourceFile file, int line, LogLevel level, const char *func);
        Logger(SourceFile file, int line, bool toAbort);
        ~Logger();

    private:
        class Impl{

        public:
            typedef  Logger::LogLevel  LogLevel;

            Timestamp time_;
            LogStream stream_;
            LogLevel level_;
            int line_;
            SourceFile basename_;

            Impl(LogLevel level, int old_errno, const SourceFile &file, int line);
            void formatTime();
            void finish();
        };

        Impl impl_;
    public:
        LogStream &stream() {return impl_.stream_;}
        static LogLevel  logLevel();
        static void setLogLevel(LogLevel level);

        typedef void (*OutputFunc) (const char *msg, int len);
        typedef void (*FlushFunc)();
        static void setOutput(OutputFunc);
        static void setFlush(FlushFunc);
        static void setTimeZone(const TimeZone &tz);


    };
    extern Logger::LogLevel g_logLevel;
    inline Logger::LogLevel Logger::logLevel() {
        return g_logLevel;
    }

    //
// CAUTION: do not write:
//
// if (good)
//   LOG_INFO << "Good news";
// else
//   LOG_WARN << "Bad news";
//
// this expends to
//
// if (good)
//   if (logging_INFO)
//     logInfoStream << "Good news";
//   else
//     logWarnStream << "Bad news";
//
#define LOG_TRACE if (salangane::Logger::logLevel() <= salangane::Logger::TRACE) \
  salangane::Logger(__FILE__, __LINE__, salangane::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (salangane::Logger::logLevel() <= salangane::Logger::DEBUG) \
  salangane::Logger(__FILE__, __LINE__, salangane::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (salangane::Logger::logLevel() <= salangane::Logger::INFO) \
  salangane::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN salangane::Logger(__FILE__, __LINE__, salangane::Logger::WARN).stream()
#define LOG_ERROR salangane::Logger(__FILE__, __LINE__, salangane::Logger::ERROR).stream()
#define LOG_FATAL salangane::Logger(__FILE__, __LINE__, salangane::Logger::FATAL).stream()
#define LOG_SYSERR salangane::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL salangane::Logger(__FILE__, __LINE__, true).stream()

    const char *strerror_tl(int savedErrno);

    // Taken from glog/logging.h
//
// Check that the input is non NULL.  This very useful in constructor
// initializer lists.

#define CHECK_NOTNULL(val) \
  ::salangane::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

    // A small helper for CHECK_NOTNULL().
    template <typename T>
    T* CheckNotNull(Logger::SourceFile file, int line, const char *names, T* ptr)
    {
        if (ptr == NULL)
        {
            Logger(file, line, Logger::FATAL).stream() << names;
        }
        return ptr;
    }
}
#endif //SALANGANE_LOGGING_H
