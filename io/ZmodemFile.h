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

class ZmodemFile
{
public:
	ZmodemFile(
	const std::string& dir, 
	const std::string& filename, 
	const std::string& fileinfo);

	~ZmodemFile();

    uint64_t getExistLen(uint32_t& crc);
	void openWrite(bool resume);
	bool write(const char* buf, unsigned long long len);
	unsigned long long getPos();
	unsigned long long getSize(){return fileSizeM;}
	unsigned long long getFileSize(){return fileSizeM;}
	unsigned long long getFileTime(){return fileTimeM;}
	bool isCompleted(){return posM == fileSizeM;}
	const std::string& getFilename(){return fileNameM;}

	ZmodemFile(const std::string& filepath,
            const std::string& basename,
            unsigned long long filesize,
            unsigned long long filetime);
	unsigned read(char*buf, unsigned size);
	void setReadPos(unsigned long long pos);
	void setWritePos(unsigned long long pos);
    uint64_t validateFileCrc(const char* fileLenAndCrc);

	bool isGood(){return fileM.good() && fileM.is_open();}

private:
	bool parseInfo(const std::string& fileinfo);

	std::fstream fileM;
	std::string fullPathM;
	std::string fileNameM;
	unsigned long long fileSizeM;
	unsigned long long fileTimeM;
	unsigned long long posM;

};

#endif /* ZMODEM_FILE_H */
