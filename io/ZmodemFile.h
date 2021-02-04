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

	bool write(const char* buf, unsigned long long len);
	unsigned long long getPos();
	unsigned long long getFileSize(){return fileSizeM;}
	unsigned long long getFileTime(){return fileTimeM;}
	bool isCompleted(){return posM == fileSizeM;}
	const std::string& getFilename(){return fileNameM;}

	ZmodemFile(const std::string& filepath,
            const std::string& basename,
            unsigned long long filesize,
            unsigned long long filetime);
	unsigned read(char*buf, unsigned size);
	void setPos(unsigned long long pos);

	bool isGood(){return fileM.good();}

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
