/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#ifndef SZ_SESSION_H
#define SZ_SESSION_H

#include "ZmodemSession.h"

class SzSession: public ZmodemSession
{
public:
    static nd::FiniteStateMachine* getZmodemFsm();

	SzSession();
	virtual ~SzSession();
    void sz(std::vector<ZmodemFile*>& files);

	void sendLeadingMsg();
	void sendZrqinit();
	void sendZfile();

private:
    std::vector<ZmodemFile*> filesM;
};

#endif /* ZMODEM_SESSION_H */
