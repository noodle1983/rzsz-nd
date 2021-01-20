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
	unsigned long long getFileSize(){return file_size_;}
	unsigned long long getFileTime(){return file_time_;}
	bool isCompleted(){return pos_ == file_size_;}
	const std::string& getFilename(){return filename_;}
	const std::string& getPrompt(){return prompt_;}
	std::string getProgressLine();

	ZmodemFile(const std::string& filepath,
            const std::string& basename,
            unsigned long long filesize,
            unsigned long long filetime);
	unsigned read(char*buf, unsigned size);
	void setPos(unsigned long long pos);

	bool isGood(){return file_.good();}

private:
	bool parseInfo(const std::string& fileinfo);

	std::fstream file_;
	std::string filename_;
	unsigned long long file_size_;
	unsigned long long file_time_;
	unsigned long long pos_;
	std::string prompt_;

};

#endif /* ZMODEM_FILE_H */
