#include "Singleton.hpp"

#include <tclap/CmdLine.h>

#include <string>
#include <vector>

class ZmodemFile;

namespace nd{

class Options{
public:
    Options()
        : debugArgM("d", "debug", "show debug messages", false, 0,"int:0-1")
    {

    }

    virtual ~Options(){}

    unsigned getDebugLevel(){return debugArgM.getValue();}
    void addCommonOptions(TCLAP::CmdLine& cmd){
        cmd.add(debugArgM);
    }

    void addFiles(TCLAP::UnlabeledMultiArg<std::string>& optionsFiles);

public:
    // common
    TCLAP::ValueArg<unsigned> debugArgM;

    // sz files
    std::vector<ZmodemFile*> filesM;
};

#define g_options nd::Singleton<nd::Options>::instance()

}

