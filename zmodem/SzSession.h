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
