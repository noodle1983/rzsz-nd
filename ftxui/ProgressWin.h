#ifndef PROGRESSWIN_H
#define PROGRESSWIN_H

#include "fsm/Singleton.hpp"
#include <ftxui/component/event.hpp>  // for Event
#include <ftxui/component/mouse.hpp>  // for ftxui
#include <ftxui/dom/elements.hpp>  // for text, separator, Element, operator|, vbox, border
#include <ftxui/dom/table.hpp>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <stdarg.h>

#include "ftxui/component/component.hpp"  // for CatchEvent, Renderer, operator|=
#include "ftxui/component/loop.hpp"       // for Loop
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive

#include "Options.h"

class ZmodemFile;

struct ProgressFileInfo{
    std::string fileM;
    uint64_t fileSizeM;
    uint64_t curSizeM;
    std::chrono::time_point<std::chrono::system_clock> beginTimeM;
    std::chrono::time_point<std::chrono::system_clock> endTimeM;
};

class ProgressWin
{
public:
    ProgressWin();

    virtual ~ProgressWin(){
        if (loopM != nullptr){
            delete loopM;
            loopM = nullptr;
        }
    }

    ftxui::Elements& addFileElements(ftxui::Elements& elements);

    void print(bool end = false);

    void addFile(ZmodemFile* zfile);
    void updateFile(ZmodemFile* zfile);
    void updateFileDone(ZmodemFile* zfile);
    void addMsg(const char* msg) { msgsM.push_back(msg); }
    void setClientWorkdingDir(const std::string& value){clientWorkingDirM = value;}
    void setServerWorkdingDir(const std::string& value){serverWorkingDirM = value;}

    void updateSentBytes(uint64_t sent){sentBytesM += sent; prevStatSentBytesM += sent;}
    void updateRecvBytes(uint64_t recv){recvBytesM += recv; prevStatRecvBytesM += recv;}

    void stat(bool force = false);

    void printReport()
    {
        showReport = true;
        stat(true);
        print(true);
    }

private:
    ftxui::ScreenInteractive screenM;
    ftxui::Loop* loopM;

    std::string clientWorkingDirM;
    std::string serverWorkingDirM;

    std::map<uint32_t, int> file2IndexMapsM;
    std::vector<ProgressFileInfo> filesM;
    std::vector<std::string> msgsM;

    std::chrono::time_point<std::chrono::system_clock> prevPrintTimeM;

    uint64_t sentBytesM;
    uint64_t recvBytesM;

    std::chrono::time_point<std::chrono::system_clock> prevStatTimeM;
    uint64_t prevStatSentBytesM;
    uint64_t prevStatRecvBytesM;

    float sentSpeedM;
    float recvSpeedM;

    float maxSentSpeedM;
    float maxRecvSpeedM;

    bool showReport;
};

#define g_progress_win (nd::Singleton<ProgressWin, 0>::instance())

#endif /* PROGRESSWIN_H */
