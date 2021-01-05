#include "SzSession.h"
#include "Processor.h"
#include "Options.h"

#include <vector>
#include <fstream>

#include <tclap/CmdLine.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

using namespace nd;
using namespace std;

void parseOption(int argc, char const *argv[]){
	try {
        TCLAP::CmdLine cmd("Send file(s) with ZMODEM protocol.", ' ', "0.1");

        TCLAP::ValueArg<unsigned> debugArg("d", "debug", "show debug messages", false, 0,"int:0-1");
        cmd.add(debugArg);

        TCLAP::UnlabeledMultiArg<string> multiArg("filepath", "files to sent", true, "string[]");
        cmd.add(multiArg);
        cmd.parse( argc, argv );

        g_options->debugLevel = debugArg.getValue();
        g_options->files = multiArg.getValue();

        for(auto f : g_options->files){
            ifstream fs(f);
            if (!fs.good()){
                cerr << "can't access file:" << f << endl;
                exit(-1);
            }
        }
	} catch (TCLAP::ArgException &e)
	{ 
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(-1);
    }
}

int main(int argc, char const *argv[])
{
    parseOption(argc, argv);

    auto session = new SzSession();
    session->sz(g_options->files);
    g_processor->run();

    return 0;
}
