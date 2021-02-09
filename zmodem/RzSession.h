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
