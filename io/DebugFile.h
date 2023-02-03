/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#ifndef DEBUG_FILE_H
#define DEBUG_FILE_H

#include <string>
#include <fstream>

class DebugFile
{
public:
	DebugFile(const std::string& filename, int lineChars = 0);
	~DebugFile();
    
    void write(const char* buff, uint32_t len);
    void switchLine();
    void close();
private:
	bool isGood(){return fileM.good() && fileM.is_open();}
    void encodeAndWrite(const char* buff, uint32_t len);

	std::fstream fileM;
	std::string fileNameM;
    int lineCharsM;
    uint64_t totalCharsM; 

};

#endif /* DEBUG_FILE_H */
