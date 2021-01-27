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
        cmd.parse( argc, argv );
	} catch (TCLAP::ArgException &e)
	{ 
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(-1);
    }
}

int main(int argc, char const *argv[])
{
    parseOption(argc, argv);
    setTtyRawMode(STDIN_FILENO);
    initZmodemTab();

    auto session = new RzSession();
    session->startInputTimer();
    g_processor->run();

    return 0;
}
