/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#include "RzSession.h"
#include "Processor.h"
#include "Options.h"

#include <vector>
#include <fstream>

#include <tclap/CmdLine.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

using namespace nd;
using namespace std;

void parseOption(int argc, char const *argv[]){
	try {
        TCLAP::CmdLine cmd("Receive file(s) with ZMODEM protocol.", ' ', "0.1");

        g_options->addCommonOptions(cmd);
        cmd.add(g_options->presetRzFilesM);

        TCLAP::SwitchArg isRzDirMode("f","rz-folder-mode","rz folder mode, make client to select folder.", cmd, false);

        cmd.parse( argc, argv );
        g_options->rzDirModeM = isRzDirMode.getValue();
	} catch (TCLAP::ArgException &e)
	{ 
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(-1);
    }
}

int main(int argc, char const *argv[])
{
    parseOption(argc, argv);
    setTtyRawMode();
    initZmodemTab();

    auto session = new RzSession();
    session->startInputTimer();
    g_processor->run();

    return 0;
}
