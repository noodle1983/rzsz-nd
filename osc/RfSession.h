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

class RfSession: public OscSession
{
public:
    static nd::FiniteStateMachine* getZmodemFsm();

	RfSession();
	virtual ~RfSession();

private:
    std::vector<ZmodemFile*> filesM;
};

#endif /* SF_SESSION_H */
