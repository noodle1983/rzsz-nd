/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#ifndef SF_SESSION_H
#define SF_SESSION_H

#include "OscSession.h"

class SfSession: public OscSession
{
public:
    static nd::FiniteStateMachine* getZmodemFsm();

	SfSession();
	virtual ~SfSession();
    void sf(std::vector<ZmodemFile*>& files);

    virtual ZmodemFile* fetchNextFileToSend()
    {
        if (filesM.empty()){return nullptr;}
        auto zmodemFile = filesM.back();
        filesM.pop_back();
        return zmodemFile;
    }
private:
    std::vector<ZmodemFile*> filesM;
};

#endif /* SF_SESSION_H */
