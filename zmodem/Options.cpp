/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#include "Options.h"
#include "ZmodemFile.h"
#include "Log.h"
#include <iostream>
#include <filesystem>
#include <map>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <cstdlib>

using namespace nd;
using namespace std;
namespace fs = filesystem;

void Options::checkEnv()
{
    const char* envTmux = std::getenv("TMUX");
    isInTmuxM = (envTmux != nullptr && strlen(envTmux) > 0);
}


void checkAddFile(const string& abPath, const string& rePath, vector<ZmodemFile*>& files)
{
    ifstream ifs(abPath);
    if (!ifs.good()){
        cerr << "can't access file:" << abPath << endl;
        exit(-1);
    }
    ifs.close();

    struct stat fileStat;
    memset(&fileStat, 0, sizeof(struct stat));
    int ret = ::stat(abPath.c_str(), &fileStat);
    if (ret != 0) {
        cerr << "can't access file:" << abPath << endl;
        exit(-1);
    }

    ZmodemFile* file  = new ZmodemFile(abPath, rePath, fileStat.st_size, fileStat.st_mtim.tv_sec);
    files.push_back(file);
    LOG_DEBUG("[checkAddFile]id:" << file->getFileId()
            << ", rePath:" << file->getFilename());
}

void Options::addFiles(TCLAP::UnlabeledMultiArg<string>& optionsFiles)
{
    // 1. the relative path
    auto files = optionsFiles.getValue();
    for(auto f : files){
        fs::path fsPath = fs::absolute(getServerWorkingDir() + "/" + f).lexically_normal();
        if(!fs::exists(fsPath)){
            cerr << "file does not exist:" << f << endl;
            exit(-1);
        }

        if(fs::is_directory(fsPath)){
            auto base = (fsPath / "..").lexically_normal();
            for(auto& p: fs::recursive_directory_iterator(fsPath)){
                auto childFsPath = p.path();
                if (!fs::is_directory(childFsPath)){
                    string abPath = childFsPath;
                    string rePath = childFsPath.lexically_relative(base);
                    checkAddFile(abPath, rePath, filesM);
                }
                else{
                    if (fs::is_empty(childFsPath)){
                        string abPath = childFsPath;
                        string rePath = childFsPath.lexically_relative(base);
                        emptyDirsM.push_back(rePath);
                        LOG_INFO("[checkAddFile]empty dir:" << rePath);
                    }
                }
            }
        }
        else{
            checkAddFile(fsPath, fsPath.filename(), filesM);
        }

    }
}

