//
// Created by root on 16-6-5.
//

#ifndef SALANGANE_CALLBACKS_H
#define SALANGANE_CALLBACKS_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "../base/Timestamp.h"

namespace salangane {
    // Adapted from google-protobuf stubs/common.h
    template<typename To, typename From>
    inline ::boost::shared_ptr<To> down_pointer_cast(const ::boost::shared_ptr<From> &f) {
        if (false) {
            // let compiler check the implict_cast which is only allowed up-cast no down-cast
            // this is more safety than static_cast
            implicit_cast<From *, To *>(0);
        }
#ifndef NDEBUG
        assert(f == NULL || dynamic_cast<To *> (get_pointer(f)) != NULL);
#endif
        return ::boost::static_pointer_cast<To>(f);

    }


    namespace net {
        // All client visible callbacks go here
        class Buffer;

        class TcpConnection;

        typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;
        typedef boost::function<void()> TimerCallback;
        typedef boost::function<void(const TcpConnectionPtr &)> ConnectionCallback;
        typedef boost::function<void(const TcpConnectionPtr &)> CloseCallback;
        typedef boost::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
        typedef boost::function<void(const TcpConnectionPtr &)> HighWaterMarkCallback;

        // the data has been read to (buf, len)
        typedef boost::function<void(const TcpConnectionPtr &,
                                     Buffer *,
                                     Timestamp)> MessageCallback;
        void defaultConnectionCallback(const TcpConnectionPtr &conn);
        void defaultMessageCallback(const TcpConnectionPtr &conn,
                                    Buffer* buffer,
                                    Timestamp receiveTime);
    }

}
#endif //SALANGANE_CALLBACKS_H
