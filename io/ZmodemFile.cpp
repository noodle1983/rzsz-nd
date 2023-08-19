/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#include "ZmodemFile.h"
#include "Log.h"
#include "crctab.h"
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

uint32_t ZmodemFile::nextFileIdM = 0; 

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
        : fileNameM(filename)
        , fileSizeM(0)
        , fileTimeM(0)
        , posM(0)
        , fileIdM(-1)
{
	parseInfo(fileinfo);

	fullPathM = dir + "/" + filename;
}

ZmodemFile::~ZmodemFile()
{
	fileM.close();
}

uint64_t ZmodemFile::getExistLen(uint32_t& crc)
{
    crc = 0xFFFFFFFFL;
	std::ifstream stream(fullPathM, std::fstream::in | std::fstream::binary);
	if (!stream.good()) { return 0; }

	uint64_t len = 0;
	char buffer[4096] = { 0 };
	do{
		stream.read(buffer, sizeof(buffer));
		unsigned readed = stream.gcount();
		for (unsigned i = 0; i < readed; i++) {
			crc = UPDC32(buffer[i], crc);
		}
		len += readed;
	} while (stream.good());
    crc = ~crc;;
	stream.close();
	return len;
}

void ZmodemFile::openWrite(bool resume)
{
    unsigned found = fullPathM.find_last_of("/\\");
	createDir(fullPathM.substr(0,found));

	if (fileM.is_open()) { fileM.close(); }
	auto exflag = resume ? std::fstream::app : std::fstream::trunc;
	fileM.open(fullPathM.c_str(), std::fstream::out|std::fstream::binary|exflag);
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

uint64_t ZmodemFile::validateFileCrc(uint64_t existLen, uint32_t existCrc)
{
    if (existLen > fileSizeM){return 0;}
    if (existLen == 0){return 0;}

    uint32_t crc = 0xFFFFFFFFL;
	std::ifstream stream(fullPathM, std::fstream::in | std::fstream::binary);
	if (!stream.good()) { return 0; }

	uint64_t len = 0;
    char buffer[4096] = { 0 };
    do{
		stream.read(buffer, sizeof(buffer));
		uint64_t readed = stream.gcount();
		for (uint64_t i = 0; i < readed && len < existLen; i++) {
			crc = UPDC32(buffer[i], crc);
            len++;
		}
	} while (stream.good());
    crc = ~crc;;
    if (crc != existCrc) {return 0;}

	return existLen;
}

ZmodemFile::ZmodemFile(const std::string& filepath, const std::string& rePath, unsigned long long filesize, unsigned long long filetime)
	: fullPathM(filepath)
	, fileNameM(rePath)
	, fileSizeM(filesize)
	, fileTimeM(filetime)
	, posM(0)
    , fileIdM(++nextFileIdM)
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
    if (!fileM.is_open()){
        fileM.open(fullPathM, std::fstream::in|std::fstream::binary);
    }
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

