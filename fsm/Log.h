#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>

class FileLogger{
public:
    FileLogger(){
        out.open("zmodem.log");
    }

    virtual ~FileLogger(){
    }

private:
    std::ofstream out;
};

//log relate
#define LOG_TRACE(msg) 
#define LOG_DEBUG(msg) 
#define LOG_INFO(msg) 
#define LOG_WARN(msg) 
#define LOG_ERROR(msg) std::cerr << msg << std::endl
#define LOG_FATAL(msg) std::cerr << msg << std::endl

#endif
