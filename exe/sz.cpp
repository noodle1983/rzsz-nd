/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#include "SzSession.h"
#include "Processor.h"
#include "Options.h"

#include <vector>
#include <fstream>

#include <tclap/CmdLine.h>

using namespace nd;
using namespace std;

void parseOption(int argc, char const *argv[]){
	try {
        TCLAP::CmdLine cmd("Send file(s) with ZMODEM protocol.", ' ', "0.1");

        TCLAP::UnlabeledMultiArg<string> multiArg("file_or_dir_path", "files/directory to sent", true, "string[]");
        cmd.add(multiArg);

        g_options->addCommonOptions(cmd);
        cmd.parse( argc, argv );

        g_options->addFiles(multiArg);
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

    auto session = new SzSession();
    session->startInputTimer();
    session->sz(g_options->filesM);
    g_processor->run();

    return 0;
}
