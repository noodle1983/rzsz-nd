#include "ZmodemFile.h"
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

	std::string full_path = dir + "/" + filename;
	unsigned found = full_path.find_last_of("/\\");
	createDir(full_path.substr(0,found));
	file_.open(full_path.c_str(), std::fstream::out|std::fstream::binary|std::fstream::trunc);
}

ZmodemFile::~ZmodemFile()
{
	file_.close();
}

bool ZmodemFile::write(const char* buf, unsigned long long len)
{
	if (!file_.is_open() || len + posM > fileSizeM){
		return 0;
	}
	file_.write(buf, len);
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

ZmodemFile::ZmodemFile(const std::string& filepath, const std::string& basename, unsigned long long filesize, unsigned long long filetime)
	: file_(filepath.c_str(), std::fstream::in|std::fstream::binary),
	fileNameM(filepath),
	fileSizeM(filesize),
	fileTimeM(filetime),
	posM(0)
{
}

unsigned ZmodemFile::read(char*buf, unsigned size)
{
	if (!file_.is_open() || !file_.good())
		return 0;
	file_.read(buf, size);
	unsigned rlen = file_.gcount();
	posM += rlen;
	return rlen;
}

void ZmodemFile::setPos(unsigned long long pos)
{
	if (pos > fileSizeM)
		return;

	file_.seekg(pos);
	posM = pos;
}

