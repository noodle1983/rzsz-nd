/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#include "RfSession.h"
#include "Processor.h"
#include "Options.h"

#include <vector>
#include <fstream>

#include <tclap/CmdLine.h>

using namespace nd;
using namespace std;

void parseOption(int argc, char const *argv[]){
	try {
        TCLAP::CmdLine cmd("Receive file(s) via OSC (Operating System Command) sequences(https://en.wikipedia.org/wiki/ANSI_escape_code#OSC).", ' ', "0.1");

        g_options->addCommonOptions(cmd);
        cmd.add(g_options->presetRzFilesM);

        TCLAP::SwitchArg isRzDirMode("f","rf-folder-mode","rf folder mode, make client to select folder.", cmd, false);

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
    g_file_logger->setExePrefix("");
    parseOption(argc, argv);
    setTtyRawMode();

    auto session = new RfSession();
    session->startInputTimer();
    g_processor->run();

    return 0;
}
