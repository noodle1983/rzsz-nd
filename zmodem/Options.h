#ifndef OPTIONS_H
#define OPTIONS_H

#include "Singleton.hpp"

#include <tclap/CmdLine.h>

#include <string>
#include <vector>

class ZmodemFile;

namespace nd{

class Options{
public:
    Options()
        : debugArgM("d", "debug", "set debug level. 0: TRACE; 1: DEBUG; 2: INFO; 3: WARN; 4: ERROR; 5: FATAL;", false, 0, "int:0-5")
        , logFileArgM("l","log", "File to log", false, "./zmodem.log", "filename") 
    {

    }

    virtual ~Options(){}

    unsigned getDebugLevel(){return debugArgM.getValue();}
    const std::string& getLogfile(){return logFileArgM.getValue();}
    void addCommonOptions(TCLAP::CmdLine& cmd){
        cmd.add(debugArgM);
        cmd.add(logFileArgM);
    }

    void addFiles(TCLAP::UnlabeledMultiArg<std::string>& optionsFiles);

public:
    // common
    TCLAP::ValueArg<unsigned> debugArgM;
    TCLAP::ValueArg<std::string> logFileArgM;

    // sz files
    std::vector<ZmodemFile*> filesM;
};

#define g_options nd::Singleton<nd::Options>::instance()

}

#endif /* OPTIONS_H */
