/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#ifndef RZ_SESSION_H
#define RZ_SESSION_H

#include "ZmodemSession.h"

class RzSession: public ZmodemSession
{
public:
    static nd::FiniteStateMachine* getZmodemFsm();

	RzSession();
	virtual ~RzSession();

	void sendLeadingMsg();
    void sendPresetFiles();

private:
};

#endif /* ZMODEM_SESSION_H */
