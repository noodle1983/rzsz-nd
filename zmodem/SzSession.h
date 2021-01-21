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

	static void sendLeadingMsg(nd::Session* session);
	static void sendZrqinit(nd::Session* session);
	static void sendZfile(nd::Session* session);
    static void handleZfileRsp(nd::Session* session);

private:
    std::vector<ZmodemFile*> filesM;
};

#endif /* ZMODEM_SESSION_H */
