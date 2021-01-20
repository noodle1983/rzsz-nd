#ifndef SZ_SESSION_H
#define SZ_SESSION_H

#include "ZmodemSession.h"

class SzSession: public ZmodemSession
{
public:
    static Fsm::FiniteStateMachine* getZmodemFsm();

	SzSession();
	virtual ~SzSession();
    void sz(std::vector<ZmodemFile*>& files);

	static void sendLeadingMsg(Fsm::Session* session);
	static void sendZrqinit(Fsm::Session* session);
	static void sendZfile(Fsm::Session* session);
    static void handleZfileRsp(Fsm::Session* session);

private:
    std::vector<ZmodemFile*> files_;
    int version_;
};

#endif /* ZMODEM_SESSION_H */
