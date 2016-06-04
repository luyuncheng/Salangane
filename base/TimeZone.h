//
// Created by root on 16-4-2.
//
#pragma once
#ifndef SALANGANE_TIMEZONE_H
#define SALANGANE_TIMEZONE_H

#include "copyable.h"
#include <memory>
#include <boost/shared_ptr.hpp>
#include <time.h>

namespace salangane {


    //TimeZone for 197~2030
    class TimeZone : public salangane::copyable {

    public:
        explicit TimeZone(const char *zonefile);
        TimeZone(int eastOfUtc, const char *tzname); // a fixed timezone
        TimeZone(){} //invalid timezone
        //~TimeZone(){delete(data_);}
        //default copy ctor/assignment/dtor are okay

        bool valid() const {
            return static_cast<bool>(data_);
        }

        struct tm toLocalTime(time_t secondsSinceEpoch) const;
        time_t fromLocalTime(const struct tm&) const;

        //gmtime(3)
        static struct tm toUtcTime(time_t secondsSinceEpoch, bool yday = false);
        //timegm(3)
        static time_t fromUtcTime(const struct tm&);
        //year in [1900,2500],month[1,12],day in [1,31]
        static time_t fromUtcTime(int year, int month, int day, int hour, int minute, int seconds);

        struct  Data;
    private:

        // in old boost it can not implict construct or delete function
        //boost::shared_ptr<Data> data_;
        std::shared_ptr<Data> data_;
    };
}
#endif //SALANGANE_TIMEZONE_H

