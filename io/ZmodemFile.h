/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#ifndef ZMODEM_FILE_H
#define ZMODEM_FILE_H

#include <string>
#include <fstream>

bool createDir(const std::string& thePath);

class ZmodemFile
{
public:
	ZmodemFile(
        const std::string& dir, 
        const std::string& filename, 
        const std::string& fileinfo);
	ZmodemFile(
        const std::string& dir, 
        const char* filename, 
        const uint64_t filesize);

	~ZmodemFile();

    uint64_t getExistLen(uint32_t& crc);
	void openWrite(bool resume);
	bool write(const char* buf, unsigned long long len);
	unsigned long long getPos() { return posM; }

	unsigned long long getSize(){return fileSizeM;}
	unsigned long long getFileSize(){return fileSizeM;}
	unsigned long long getFileTime(){return fileTimeM;}
    uint32_t getFileId(){return fileIdM;}
    void setFileId(uint32_t fileId){fileIdM = fileId;}
	bool isCompleted(){return posM == fileSizeM;}
	const std::string& getFilename(){return fileNameM;}

	ZmodemFile(const std::string& filepath,
            const std::string& basename,
            unsigned long long filesize,
            unsigned long long filetime);
	unsigned read(char*buf, unsigned size);
	void setReadPos(unsigned long long pos);
	void setWritePos(unsigned long long pos);
    uint64_t validateFileCrc(uint64_t existLen, uint32_t existCrc);

	bool isGood(){return fileM.good() && fileM.is_open();}

private:
	bool parseInfo(const std::string& fileinfo);

	std::fstream fileM;
	std::string fullPathM;
	std::string fileNameM;
	unsigned long long fileSizeM;
	unsigned long long fileTimeM;
	unsigned long long posM;

    uint32_t fileIdM; 
    static uint32_t nextFileIdM; 
};

#endif /* ZMODEM_FILE_H */
