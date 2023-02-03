/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#ifndef OPTIONS_H
#define OPTIONS_H

#include "Singleton.hpp"

#include <tclap/CmdLine.h>

#include <string>
#include <vector>

#ifdef DEBUG
const unsigned DEFAULT_LOG_LEVEL = 0;
#else
const unsigned DEFAULT_LOG_LEVEL = 4;
#endif

class ZmodemFile;

namespace nd{

class Options{
public:
    Options()
        : debugArgM("d", "debug", "set debug level. 0: TRACE; 1: DEBUG; 2: INFO; 3: WARN; 4: ERROR; 5: FATAL;",
                false, DEFAULT_LOG_LEVEL, "int:0-5")
        , logFileArgM("l","log", "File to log", false, "./zmodem.log", "filename") 
        , clientWorkingDirM("c","client-working-dir", "Depending on ZVERSION:1, set the working directory on the client side. It is desktop if not set.",
                false, "", "directory path") 
        , serverWorkingDirM("s","server-working-dir", "Depending on ZVERSION:1, set the working directory on the server side. It is current directory if not set.",
                false, "./", "directory path") 
        , logDataM("t","test-data", "Log Test Data", false) 
        , rzDirModeM(false)
        , presetRzFilesM("p","preset-files", "Depending on ZVERSION:1, preset rz files splited by '#' to skip file selection",
                false, "", "directory/file path") 
    {

    }

    virtual ~Options(){}

    unsigned getDebugLevel(){return debugArgM.getValue();}
    const std::string& getLogfile(){return logFileArgM.getValue();}
    const std::string& getClientWorkingDir(){return clientWorkingDirM.getValue();}
    const std::string& getServerWorkingDir(){return serverWorkingDirM.getValue();}
    bool shouldLogTestData(){return logDataM.getValue();}
    const std::string& getPresetRzFiles(){return presetRzFilesM.getValue();}
    void addCommonOptions(TCLAP::CmdLine& cmd){
        cmd.add(debugArgM);
        cmd.add(logFileArgM);
        cmd.add(clientWorkingDirM);
        cmd.add(serverWorkingDirM);
        cmd.add(logDataM);
    }

    void addFiles(TCLAP::UnlabeledMultiArg<std::string>& optionsFiles);

public:
    // common
    TCLAP::ValueArg<unsigned> debugArgM;
    TCLAP::ValueArg<std::string> logFileArgM;
    TCLAP::ValueArg<std::string> clientWorkingDirM;
    TCLAP::ValueArg<std::string> serverWorkingDirM;
    TCLAP::SwitchArg logDataM;

    // sz files
    std::vector<ZmodemFile*> filesM;

    // rz files
    bool rzDirModeM;
    TCLAP::ValueArg<std::string> presetRzFilesM;
};

#define g_options nd::Singleton<nd::Options>::instance()

}

#endif /* OPTIONS_H */
