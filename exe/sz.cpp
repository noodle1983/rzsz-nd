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

        TCLAP::UnlabeledMultiArg<string> multiArg("filepath", "files to sent", true, "string[]");
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
    setTtyRawMode(STDIN_FILENO);
    initZmodemTab();

    auto session = new SzSession();
    session->startInputTimer();
    session->sz(g_options->filesM);
    g_processor->run();

    return 0;
}
