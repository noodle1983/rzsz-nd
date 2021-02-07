#include "ZmodemFile.h"
#include "Log.h"
#include "crctab.h"
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

void createDir(const std::string& thePath)
{
    if (thePath.empty())
        return;

    size_t pos = 0;
	while ((pos = thePath.find_first_of("/\\", pos)) != std::string::npos) 
    {
        std::string preDir = thePath.substr(0, pos);
        pos++;
        DIR* dir = opendir(preDir.c_str());
        if (dir != NULL)
        {
            closedir(dir);
            continue;
        }
        if (errno != ENOENT)
        {
            return;
        }

        mkdir(preDir.c_str(), 0774); 
    }
    DIR* dir = opendir(thePath.c_str());
    if (dir != NULL)
    {
        closedir(dir);
        return;
    }
    if (errno != ENOENT)
    {
        return;
    }
    mkdir(thePath.c_str(), 0774); 
}


ZmodemFile::ZmodemFile(
	const std::string& dir, 
	const std::string& filename, 
	const std::string& fileinfo)
	: fileNameM(filename),
	fileSizeM(0),
	fileTimeM(0),
	posM(0)
{
	parseInfo(fileinfo);

	fullPathM = dir + "/" + filename;
	unsigned found = fullPathM.find_last_of("/\\");
	createDir(fullPathM.substr(0,found));
	fileM.open(fullPathM.c_str(), std::fstream::out|std::fstream::binary|std::fstream::trunc);
}

ZmodemFile::~ZmodemFile()
{
	fileM.close();
}

bool ZmodemFile::write(const char* buf, unsigned long long len)
{
	if (!fileM.is_open() || len + posM > fileSizeM){
        LOG_ERROR("write failed! errno:" << errno);
		return 0;
	}
    long long pos = fileM.tellp();
    fileM.write(buf, len);
	long long written = (long long)fileM.tellp() - pos;
    if (written != (long long)len){
        LOG_ERROR("write not match. input len:" << len << ", written:" << written);
		return 0;
    }
	posM += len;
	return 1;
}

unsigned long long ZmodemFile::getPos()
{
	return posM;
}

bool ZmodemFile::parseInfo(const std::string& fileinfo)
{
	unsigned st_mode = 0;
	int file_left = 0;
	unsigned long  left_total = 0;
	fileSizeM = 0;
	fileTimeM = 0;
	sscanf(fileinfo.c_str(), "%llu %llo %o 0 %d %ld", &fileSizeM, &fileTimeM, &st_mode, &file_left, &left_total);
	return 1;
}

uint64_t ZmodemFile::validateFileCrc(const char* fileLenAndCrc)
{
    uint64_t existLen = 0;
    uint32_t existCrc = 0;
	sscanf(fileLenAndCrc, "%llu %u", (unsigned long long*)&existLen, &existCrc);

    if (existLen > fileSizeM){return 0;}

    uint32_t crc = 0xFFFFFFFFL;
	std::ifstream stream(fullPathM, std::fstream::in | std::fstream::binary);
	if (!stream.good()) { return 0; }

	uint64_t len = 0;
	unsigned char ch = 0;
	while (stream.read((char*)&ch, 1) && len < existLen) {
		crc = UPDC32(ch, crc);
		len++;
	}
    crc = ~crc;;
    if (crc != existCrc) {return 0;}

	return existLen;
}

ZmodemFile::ZmodemFile(const std::string& filepath, const std::string& rePath, unsigned long long filesize, unsigned long long filetime)
	: fileM(filepath.c_str(), std::fstream::in|std::fstream::binary),
	fullPathM(filepath),
	fileNameM(rePath),
	fileSizeM(filesize),
	fileTimeM(filetime),
	posM(0)
{
}

unsigned ZmodemFile::read(char*buf, unsigned size)
{
	if (!fileM.is_open() || !fileM.good())
		return 0;
	fileM.read(buf, size);
	unsigned rlen = fileM.gcount();
	posM += rlen;
	return rlen;
}

void ZmodemFile::setReadPos(unsigned long long pos)
{
	if (pos > fileSizeM)
		return;

	fileM.seekg(pos);
	posM = pos;
}

void ZmodemFile::setWritePos(unsigned long long pos)
{
	if (pos > fileSizeM)
		return;

	posM = pos;
}

