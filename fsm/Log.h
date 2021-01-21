#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>
#include <thread>

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "fsm/Singleton.hpp"
class FileLogger{
public:
    FileLogger(){
        outM.open("zmodem.log");
    }

    std::ofstream& stream(const char* levelStr, const char* file, const unsigned lineno){
        time_t rawTime;
        struct tm info;
        char timeStr[80];

        time(&rawTime);
        localtime_r(&rawTime, &info);
        strftime(timeStr, 80, "%Y-%m-%d %H:%M:%S ", &info);

        outM << timeStr << levelStr;// << "(" << file << ":" << lineno << ")";
        return outM;
    }

    std::mutex& mutex(){return mutexM;}

    virtual ~FileLogger(){
        outM.flush();
    }

private:
    std::ofstream outM;
    std::mutex mutexM;
};

#define g_file_logger (nd::Singleton<FileLogger, 0>::instance())
#define LOG(level, toErr, msg) {\
    std::lock_guard<std::mutex> lock(g_file_logger->mutex()); \
    g_file_logger->stream(level, __FILE__, __LINE__) << msg << std::endl; \
    if(toErr){std::cerr << msg << std::endl;} \
}

//log relate
#define LOG_TRACE(msg) LOG("TRACE ", false, msg)
#define LOG_DEBUG(msg) LOG("DEBUG ", false, msg)
#define LOG_INFO(msg)  LOG("INFO  ", false, msg)
#define LOG_WARN(msg)  LOG("WARN  ", false, msg)
#define LOG_ERROR(msg) LOG("ERR   ", true, msg)
#define LOG_FATAL(msg) LOG("FATAL ", true, msg)

#endif
