//
// Created by root on 16-5-31.
//

#ifndef SALANGANE_FILEUTIL_H
#define SALANGANE_FILEUTIL_H

#include "StringPiece.h"
#include <boost/noncopyable.hpp>

namespace salangane {
    namespace FileUtil {
#define kBufferSize  64 * 1024
        // read small file < 64kB
        class ReadSmalFile : boost::noncopyable {
        public:
            ReadSmalFile(StringArg filename);
            ~ReadSmalFile();

            //return errno
            template <typename Str>
            int readToString (int maxSize,
                              Str *content,
                              int64_t *fileSize,
                              int64_t *modifyTime,
                              int64_t *createTime);

            // Read at maxium kBufferSize into buf_
            // else return errno
            int readToBuffer(int *size);

            const char *buffer() const {
                return buf_;
            }

            //static const kBufferSize = 64 * 1024;
        private:
            int fd_;
            int err_;
            char buf_[kBufferSize];
        };
        // read the file content, returns errno if error happens
        template <typename Str>
        int readFile (StringArg filename,
                      int maxSize,
                      Str *content,
                      int64_t *fileSize = NULL,
                      int64_t *modifyTime = NULL,
                      int64_t *createTime = NULL)
        {
            ReadSmalFile file(filename);
            return file.readToString(maxSize,content,fileSize,modifyTime,createTime);
        }

        // not thread safe
        class AppendFile : boost::noncopyable {
        public:
            explicit AppendFile(StringArg filename);

            ~AppendFile();

            void append(const char *logline, const size_t len);

            void flush();

            size_t writtenBytes() const { return writtenBytes_; }
        private:
            size_t write(const char *logline, size_t len);
            FILE *fp_;
            //char buffer_[64*1024];
            char buffer_ [kBufferSize];
            size_t writtenBytes_;
        };
    }
}

#endif //SALANGANE_FILEUTIL_H
