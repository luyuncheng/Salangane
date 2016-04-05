//
// Created by root on 16-3-30.
//

#ifndef SALANGANE_TYPES_H
#define SALANGANE_TYPES_H

#include <cstdint>

#ifdef SALANGANE_STD_STRING
#include <string>
#else //!SALANGANE_STD_STRING
#include <ext/vstring.h>
#include <ext/vstring_fwd.h>
#endif

#ifndef NDEBUG
#include <cassert>
#endif



namespace salangane {

#ifdef SALANGANE_STD_STRING
using std::string;
#else
typedef __gnu_cxx::__sso_string string;
#endif

    template <typename  To,typename  From>
    inline To implicit_cast(From const &f) {
        return f;
    }

    //这里的if(false)说明编译器不要执行,但是编译器会去检查里面的implicit_cast推导出来是否合法
    //要编译器提供报警的机制
    template <typename  To,typename From>
    inline To down_cast(From *f) {
        if (false) {
            //说明是要编译器检查是否有down_cast的implicit_cast
            implicit_cast<From *, To>(0);
        }
#if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
        assert(f == NULL || dynamic_cast<To>(f) != NULL);  // RTTI: debug mode only!
#endif
        return static_cast<To>(f);
    }
}

#endif //SALANGANE_TYPES_H


