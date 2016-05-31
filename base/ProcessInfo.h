//
// Created by root on 16-5-31.
//

#ifndef SALANGANE_PROCESSINFO_H
#define SALANGANE_PROCESSINFO_H

#include "StringPiece.h"
#include "Types.h"
#include "Timestamp.h"
#include <vector>

namespace salangane {
    namespace ProcessInfo {
        pid_t pid();
        string pidString();
        uid_t uid();
        string username();
        uid_t euid();

        Timestamp startTime();
        int clockTicksPerSecond();
        int pageSize();
        bool isDebugBuild(); //const expr

        string hostname();
        string procname();
        StringPiece procname(const string & stat);

        // cat /proc/self/status
        string procStatus();

        // cat /proc/self/self/stat
        string procStat();

        // cat /proc/self/task/tid/stat  every thread stat
        string threadstat();

        // readlink /proc/self/exe
        string exePath();

        int openedFiles();
        int maxOpenFiles();

        struct CpuTime {
            double userSeconds;
            double systemSeconds;

            CpuTime() : userSeconds(0.0), systemSeconds(0.0) { }

        };

        CpuTime cpuTime();

        int numThreads();
        std::vector<pid_t> threads();

    }
}

#endif //SALANGANE_PROCESSINFO_H
