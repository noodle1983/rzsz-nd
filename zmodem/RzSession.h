#ifndef RZ_SESSION_H
#define RZ_SESSION_H

#include "ZmodemSession.h"

class RzSession: public ZmodemSession
{
public:
    static nd::FiniteStateMachine* getZmodemFsm();

	RzSession();
	virtual ~RzSession();

	static void sendLeadingMsg(nd::Session* session);

private:
};

#endif /* ZMODEM_SESSION_H */
