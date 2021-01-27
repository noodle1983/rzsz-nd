#include "Options.h"
#include "ZmodemFile.h"
#include <iostream>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

using namespace nd;
using namespace std;

void Options::addFiles(TCLAP::UnlabeledMultiArg<string>& optionsFiles)
{
    auto files = optionsFiles.getValue();
    for(auto f : files){
        struct stat fileStat;
        memset(&fileStat, 0, sizeof(struct stat));
        int ret = ::stat(f.c_str(), &fileStat);
        if (ret != 0) {
            cerr << "can't access file:" << f << endl;
            exit(-1);
        }

        ifstream fs(f);
        if (!fs.good()){
            cerr << "can't access file:" << f << endl;
            exit(-1);
        }

        ZmodemFile* file  = new ZmodemFile(f, f, fileStat.st_size, fileStat.st_mtim.tv_sec);
        filesM.push_back(file);
    }
}

