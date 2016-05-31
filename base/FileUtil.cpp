//
// Created by root on 16-5-31.
//

#include "FileUtil.h"
#include "Logging.h"

#include <boost/static_assert.hpp>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

using namespace salangane;

FileUtil::AppendFile::AppendFile(StringArg filename)
    : fp_(::fopen(filename.c_str(), "ae")), // 'e' for O_CLOEXEC
      writtenBytes_(0)
{
    assert(fp_);
    // If BUF is NULL, make STREAM unbuffered.
    //Else make it use SIZE bytes of BUF for buffering.
    ::setbuffer(fp_, buffer_, sizeof(buffer_));
}

FileUtil::AppendFile::~AppendFile() {
    ::fclose(fp_);
}

void FileUtil::AppendFile::append(const char *logline, const size_t len) {
    size_t n = write(logline, len);
    size_t remain = len - n;
    while(remain > 0) {
        size_t x = write(logline + n, remain);
        if(x == 0) {
            int err = ferror(fp_);
            if(err) {
                fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tl(err));
            }
            break;
        }
        n += x;
        remain = len - n; // remain -= x
    }
    writtenBytes_ += len;
}

void FileUtil::AppendFile::flush() {
    ::fflush(fp_);
}

size_t FileUtil::AppendFile::write(const char *logline, size_t len) {
    // undef fwrite_unlocked
    return ::fwrite_unlocked(logline, 1, len, fp_);
}

FileUtil::ReadSmalFile::ReadSmalFile(StringArg filename)
    : fd_ (::open(filename.c_str(), O_RDONLY | O_CLOEXEC)),
      err_(0)
{
    buf_[0] = '\0';
    if(fd_ < 0) {
        err_ = errno;
    }
}

FileUtil::ReadSmalFile::~ReadSmalFile() {
    if(fd_ >= 0) {
        ::close(fd_);
    }
}

//return errno
// [modifyTIme] [creatTime] selective argument if you want to know these time you can know
template <typename Str>
int FileUtil::ReadSmalFile::readToString(int maxSize,
                                         Str *content,
                                         int64_t *fileSize,
                                         int64_t *modifyTime,
                                         int64_t *createTime)
{
    // off_t is default as long to define offset of file
    BOOST_STATIC_ASSERT(sizeof(off_t) == 8);
    assert(content != NULL);
    int err = err_;
    if(fd_ >= 0) {
        content->clear();

        if(fileSize) {
            struct stat statbuf;
            //
            // int fstat (int fd, struct stat *buf)
            // is to return the file's state to struct stat buf
            // if success return 0 else return -1 error code stored in errno
            //
            if(::fstat(fd_, &statbuf) == 0) {
                // S_ISREG judge whether is normal file
                // S_ISLINK judge whether is link
                // S_ISDIR judge whether is dir
                // S_ISCHR judge whether is a character device
                // S_ISBLK judge whether is block device
                // S_ISFIFO judge whether is FIFO file
                // S_ISSOCK judge whether is socket file
                if(S_ISREG(statbuf.st_mode)) {
                    *fileSize = statbuf.st_size;
                    content->reserve(static_cast<int>(std::min(implicit_cast<int64_t>(maxSize), *fileSize)));
                } else if(S_ISDIR(statbuf.st_mode)) {
                    err = EISDIR;
                }
                if(modifyTime) {
                    *modifyTime = statbuf.st_mtim.tv_nsec;
                }
                if(createTime) {
                    *createTime = statbuf.st_ctim.tv_nsec;
                }
            } else {
                err = errno;
            }
        }

        while(content->size() < implicit_cast<size_t>(maxSize)) {
            size_t toRead = std::min(implicit_cast<size_t>(maxSize) - content->size(),
                                     sizeof(buf_));
            ssize_t n = ::read(fd_, buf_, toRead);
            if(n > 0) {
                content->append(buf_, n);
            } else {
                if(n < 0) {
                    err = errno;
                }
                break;
            }
        }
    }
    return err;
}

// readTobuffer and give the n bytes to size
// return is errno
int FileUtil::ReadSmalFile::readToBuffer(int *size) {
    int err = err_;
    if (fd_ >= 0) {

//        Read NBYTES into BUF from FD at the given position OFFSET without changing the file pointer.  Return the number read, -1 for erroror 0 for EOF.
        ssize_t n = ::pread(fd_, buf_, sizeof(buf_)-1, 0);
        if(n >= 0) {
            if(size) {
                *size = static_cast<int>(n);
            }
            buf_[n] = '\0';
        } else {
            err = errno;
        }
    }
    return err;
}


template int FileUtil::readFile (StringArg filename,
                        int maxSize,
                        string *content,
                        int64_t *,
                        int64_t *,
                        int64_t *) ;

template int FileUtil::ReadSmalFile::readToString(int maxSize,
                                                  string *content,
                                                  int64_t *,
                                                  int64_t *,
                                                  int64_t *);

#ifndef SALANGANE_STD_STRING

template int FileUtil::readFile (StringArg filename,
                                 int maxSize,
                                 std::string *content,
                                 int64_t *,
                                 int64_t *,
                                 int64_t *) ;

template int FileUtil::ReadSmalFile::readToString(int maxSize,
                                                  std::string *content,
                                                  int64_t *,
                                                  int64_t *,
                                                  int64_t *);

#endif
// error code
//错误代码:
//ENOENT 参数file_name指定的文件不存在
//ENOTDIR 路径中的目录存在但却非真正的目录
//ELOOP 欲打开的文件有过多符号连接问题，上限为16符号连接
//EFAULT 参数buf为无效指针，指向无法存在的内存空间
//        EACCESS 存取文件时被拒绝
//ENOMEM 核心内存不足
//ENAMETOOLONG 参数file_name的路径名称太长
//
//int stat(const char *file_name,struct stat *buf);
//int fstat(int filedes,struct stat *buf);
//stat 结构定义于：/usr/include/sys/stat.h 文件中
//　　struct stat finfo;
//　　stat( sFileName, &finfo );
//　　int size = finfo. st_size;
//　　struct stat {
//    　　mode_t st_mode; //文件对应的模式，文件，目录等
//    　　ino_t st_ino; //i-node节点号
//    　　dev_t st_dev; //设备号码
//    　　dev_t st_rdev; //特殊设备号码
//    　　nlink_t st_nlink; //文件的连接数
//    　　uid_t st_uid; //文件所有者
//    　　gid_t st_gid; //文件所有者对应的组
//    　　off_t st_size; //普通文件，对应的文件字节数
//    　　time_t st_atime; //文件最后被访问的时间
//    　　time_t st_mtime; //文件内容最后被修改的时间
//    　　time_t st_ctime; //文件状态（属性）改变时间
//    　　blksize_t st_blksize; //文件内容对应的块大小
//    　　blkcnt_t st_blocks; //文件内容对应的块数量
//    　　};
