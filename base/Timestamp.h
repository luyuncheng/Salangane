//
// Created by root on 16-4-2.
//

#ifndef SALANGANE_TIMESTAMP_H
#define SALANGANE_TIMESTAMP_H

#include "copyable.h"
#include "Types.h"

#include <boost/operators.hpp>

namespace salangane {
    ///
/// Time stamp in UTC, in microseconds resolution.
///
/// This class is immutable.
/// It's recommended to pass it by value, since it's passed in register on x64.
    class Timestamp : public salangane::copyable,
                      public boost::less_than_comparable<Timestamp> {
    private:
        int64_t microSecondSinceEpoch_;
    public:
        static const int kMicroSecondsPerSecond = 1000 * 1000;
        Timestamp():microSecondSinceEpoch_(0){}
        ///
        /// Constucts a Timestamp at specific time
        ///
        /// @param microSecondsSinceEpoch

        explicit Timestamp(int64_t microSecondSinceEpochArg) : microSecondSinceEpoch_(microSecondSinceEpochArg){}
        void swap (Timestamp & that) {
            std::swap(microSecondSinceEpoch_,that.microSecondSinceEpoch_);
        }
        // default copy/assignment/dtor are Okay

        string toString() const;
        string toFormattedString(bool showMicroseconds = true) const;

        bool valid() const { return microSecondSinceEpoch_ > 0;}

//        for internal usage
        int64_t  microSecondsSinceEpoch() const { return microSecondSinceEpoch_;}
        time_t  secondsSinceEpoch() const {
            return static_cast<time_t> (microSecondSinceEpoch_ / kMicroSecondsPerSecond);
        }
        //
        //Get time Now
        //
        static Timestamp now();
        static Timestamp invalid() {return Timestamp();}

        static Timestamp fromUnixTime(time_t t) {

            return fromUnixTime(t,0);
        }

        static Timestamp fromUnixTime(time_t t, int microseconds) {
            return Timestamp(static_cast<int64_t >(t) * kMicroSecondsPerSecond +microseconds);
        }


    };

    inline bool operator < (Timestamp lhs, Timestamp rhs) {

        return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
    }

    inline bool operator == (Timestamp lhs, Timestamp rhs) {
        return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
    }

    ///
/// Gets time difference of two timestamps, result in seconds.
///
/// @param high, low
/// @return (high-low) in seconds
/// @c double has 52-bit precision, enough for one-microsecond
/// resolution for next 100 years.
    inline double timeDifference(Timestamp high, Timestamp low)
    {
        int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
        return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
    }

///
/// Add @c seconds to given timestamp.
///
/// @return timestamp+seconds as Timestamp
///
    inline Timestamp addTime(Timestamp timestamp, double seconds)
    {
        int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
        return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
    }
}
#endif //SALANGANE_TIMESTAMP_H
