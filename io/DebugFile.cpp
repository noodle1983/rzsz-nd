#include "DebugFile.h"
#include "Log.h"
#include "zmodem.h"

DebugFile::DebugFile(const std::string& filename, int lineChars)
{
    fileNameM = filename;
    lineCharsM = lineChars;
    totalCharsM = 0;
}

DebugFile::~DebugFile()
{
    if (isGood()){
        fileM.close();
    }
}

void DebugFile::encodeAndWrite(const char* buff, uint32_t len)
{
    for(uint32_t i = 0; i < len; i++){
        totalCharsM++;
        char hex[3] = {0};
        unsigned char c = (unsigned char)buff[i];
        hex[0] = HEX_ARRAY[(c&0xF0) >> 4];
        hex[1] = HEX_ARRAY[c&0x0F];
        hex[2] = ' ';
        fileM.write(hex, 3);
        if (lineCharsM > 0){
            if ((totalCharsM % lineCharsM) == 0){
                switchLine();
            }

        }
    }
}

void DebugFile::write(const char* buf, uint32_t len)
{
    if (isGood()){
        encodeAndWrite(buf, len);
        return;
    }

	auto flag = std::fstream::out | std::fstream::binary | std::fstream::trunc;
	fileM.open(fileNameM.c_str(), flag);
    if (!isGood()) {
        LOG_ERROR("failed to open file[" << fileNameM << "] errno:" << errno);
        return;
    }
    totalCharsM = 0;

    encodeAndWrite(buf, len);
}

void DebugFile::switchLine()
{
    if (isGood()){
        fileM.seekp(-1, std::ios_base::cur);
        fileM << std::endl;
    }
}

void DebugFile::close()
{
    if (isGood()){
        fileM.close();
    }
}

