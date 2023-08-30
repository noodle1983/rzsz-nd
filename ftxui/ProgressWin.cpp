#include "ProgressWin.h"

#include "ZmodemFile.h"

static std::string format(const char* str, ...)
{
    char buffer[1024] = { 0 };
    va_list ap;
    va_start(ap, str);
    vsnprintf(buffer, sizeof(buffer), str, ap);
    va_end(ap);
    return std::string(buffer);
}

const static int GB = 1024 * 1024 * 1024;
const static int MB = 1024 * 1024;
const static int KB = 1024;
static std::string formatSpeed(uint64_t bytes)
{
    if (bytes >= GB){
        return format("%.2fGB/s", bytes * 1.0f / GB);
    }
    else if (bytes >= MB){
        return format("%.2fMB/s", bytes * 1.0f / MB);
    }
    else if (bytes >= KB){
        return format("%.2fKB/s", bytes * 1.0f / KB);
    }
    return format("%dB/s", bytes);
}

//-----------------------------------------------------------------------------

ProgressWin::ProgressWin()
    : screenM(ftxui::ScreenInteractive::FitComponent())
    , sentBytesM(0)
    , recvBytesM(0)
    , prevStatTimeM(std::chrono::system_clock::now())
    , prevStatSentBytesM(0)
    , prevStatRecvBytesM(0)
    , sentSpeedM(0)
    , recvSpeedM(0)
    , maxSentSpeedM(0)
    , maxRecvSpeedM(0)
    , showReport(false)
{
    using namespace ftxui;
    auto component = Renderer([&] {
        auto header = g_options->getExeName() + ": local[" + g_options->getClientWorkingDir() + "]"
            + (g_options->isDownload()? " <- " : " -> ")
            + "remote[" + g_options->getServerWorkingDir() + "]";

        Elements elements = { hbox({
                text(header),
            }),
        };

        if (recvSpeedM > 0 || sentSpeedM > 0){
            elements.emplace_back(hbox({
                text("Speed:r:"),
                text(formatSpeed(recvSpeedM)),
                filler(),
                text("s:"),
                text(formatSpeed(sentSpeedM)),
            }));
        }

        addFileElements(elements);

        for(auto& msg: msgsM){
            elements.push_back(text(msg));
        }

        return vbox(elements);
    });

    loopM = new ftxui::Loop(&screenM, component);
}

//-----------------------------------------------------------------------------

ftxui::Elements& ProgressWin::addFileElements(ftxui::Elements& elements)
{
    using namespace ftxui;
    if(filesM.empty()){
        return elements;
    }
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    float speed = g_options->isDownload() ? sentSpeedM : recvSpeedM;

    std::vector<Elements> fileGrids;
    fileGrids.emplace_back(Elements({
        text("File"), 
        separator(),
        text((g_options->isDownload() ? "Sent/Total" : "Recv/Total")), 
        separator(),
        text("Used/Left Time"),
        separator(),
        text("Progress"),
    }));

    for(int i = filesM.size() - 1; i >= 0 && (showReport || i >= (int)filesM.size() - 3); i--){
        auto& file = filesM[i];
        bool isComplete = file.curSizeM == file.fileSizeM;
        std::chrono::duration<float> diff = (isComplete && file.endTimeM > file.beginTimeM)? (file.endTimeM - file.beginTimeM) : (now - file.beginTimeM);
        float eta = 0;
        if (speed > 0){
            eta = (file.fileSizeM - file.curSizeM) / speed;
        }
        fileGrids.emplace_back(Elements({separator(), separator(), separator(), separator(), separator(), separator(), separator()}));
        fileGrids.emplace_back(Elements({
            text(file.fileM),
            separator(),
            text(format("%ld/%ld", file.curSizeM, file.fileSizeM)),
            separator(),
            text(speed > 0 ? format("%.1fs/%.1fs", diff.count(), eta) : format("%.1fs/---", diff.count())),
            separator(),
            gauge(file.fileSizeM > 0 ? file.curSizeM * 1.0f / file.fileSizeM : 1),
        }));

    }
    elements.push_back(gridbox(fileGrids) | border );

    return elements;
}

//-----------------------------------------------------------------------------

void ProgressWin::print(bool end)
{
    if (loopM == nullptr) { return; }

    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    const std::chrono::duration<float> diff = (now - prevPrintTimeM);
    if (!end && (diff.count() < 0.2f)){ return; }
    prevPrintTimeM = now;

    loopM->RunOnce();
}

//-----------------------------------------------------------------------------

void ProgressWin::addFile(ZmodemFile* zfile)
{
    stat();

    auto it = file2IndexMapsM.find(zfile->getFileId()); 
    ProgressFileInfo* fileInfo = nullptr;
    if (it == file2IndexMapsM.end()){
        auto index = filesM.size();
        file2IndexMapsM[zfile->getFileId()] = index;
        filesM.push_back(ProgressFileInfo());
        fileInfo = &filesM[index];

        fileInfo->fileM = zfile->getFilename();
        fileInfo->fileSizeM = zfile->getFileSize();
        fileInfo->beginTimeM = std::chrono::system_clock::now();
    }
    else {
        fileInfo = &filesM[it->second];
    }
    fileInfo->curSizeM = zfile->getPos();

    print();
}

//-----------------------------------------------------------------------------

void ProgressWin::updateFile(ZmodemFile* zfile)
{
    stat();

    auto it = file2IndexMapsM.find(zfile->getFileId()); 
    ProgressFileInfo* fileInfo = nullptr;
    if (it == file2IndexMapsM.end()){
        return;
    }

    fileInfo = &filesM[it->second];
    fileInfo->curSizeM = zfile->getPos();

    print();
}

//-----------------------------------------------------------------------------

void ProgressWin::updateFileDone(ZmodemFile* zfile)
{
    stat();

    auto it = file2IndexMapsM.find(zfile->getFileId()); 
    ProgressFileInfo* fileInfo = nullptr;
    if (it == file2IndexMapsM.end()){
        return;
    }

    fileInfo = &filesM[it->second];
    fileInfo->curSizeM = fileInfo->fileSizeM;
    fileInfo->endTimeM = std::chrono::system_clock::now();

    print();
}

//-----------------------------------------------------------------------------

void ProgressWin::stat(bool force)
{
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    const std::chrono::duration<float> diff = (now - prevStatTimeM);
    if (!force && (diff.count() < 1)){ return; }

    prevStatTimeM = now;
    sentSpeedM = prevStatSentBytesM * 1.0f / diff.count();
    recvSpeedM = prevStatRecvBytesM * 1.0f / diff.count();
    if (sentSpeedM > maxSentSpeedM){ maxSentSpeedM = sentSpeedM;}
    if (recvSpeedM > maxRecvSpeedM){ maxRecvSpeedM = recvSpeedM;}

    prevStatSentBytesM = 0;
    prevStatRecvBytesM = 0;
}

//-----------------------------------------------------------------------------

