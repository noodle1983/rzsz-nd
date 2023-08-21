/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#ifndef OSC_SESSION_H
#define OSC_SESSION_H

#include "Fsm.h"
#include "Action.h"
#include "State.h"
#include "Session.h"
#include "zmodem.h"
#include "ZmodemFile.h"
#include "DebugFile.h"
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <fstream>
#include <vector>
#include <string.h>
#include <map>

namespace nd {
    struct OscPkg;
}

class OscSession: public nd::Session
{
public:
	enum MyState
	{
		INIT_STATE = 1,
        SEND_FILE_INFO_STATE,
        INIT_RECV_STATE,
		IDLE_STATE,
		SEND_DATA_STATE,
		WAIT_DATA_STATE,
		HANDLE_DATA_STATE,
		DESTROY_STATE,
		END_STATE
	};
	enum MyEvent
	{
		NETWORK_INPUT_EVT = 0,
		RESET_EVT,
		SEND_DATA_EVT,
		SEND_DATA_LATER_EVT,
		DESTROY_EVT,
		SKIP_EVT,
		SEND_FILE_EVT,
	};

	OscSession(nd::FiniteStateMachine* fsm);
	virtual ~OscSession();
	void initState();
    void startInputTimer();
    void stopInputTimer();
    static void onInputTimerout(void *arg);
	int processNetworkInput(const char* const str, const int len);
	void reset();
	void destroy();
	
	void parsePkg();
    void waitZdata();
    void sendClientWorkingDir();

	static void deleteSelf(nd::Session* session);

	const char* curBuffer(){return bufferM + decodeIndexM;}
	void eatBuffer(){
        // this will make all the pos and index in buffer invalid
        bufferLenM -= decodeIndexM;
        for(unsigned i = 0; i < bufferLenM; i++){ bufferM[i] = bufferM[decodeIndexM + i];}
        memset(bufferM + bufferLenM, 0, sizeof(bufferM) - bufferLenM);
		decodeIndexM = 0;
	}

	void sendInitReq();
    void sendFileInfo();
    void handleFileProposeStart(const nd::OscPkg *pkg);
    void sendFileInitPos(ZmodemFile* zmodemFile);
    void sendFileContent();
    void sendFileComplete(ZmodemFile* zmodemFile);
    void handleFileCompleteAck(const nd::OscPkg *pkg);

    void sendInitRecv();
    void handleFileInfo(const nd::OscPkg* pkg);
    void handleFileContent(const nd::OscPkg* pkg);
    void handleFileInitPos(const nd::OscPkg* pkg);
    void handleFileComplete(const nd::OscPkg* pkg);
    void sendFileCompleteAck(uint32_t fileId);
	void sendBye();
	void sendByeBye();
protected:
    void sendPkg(const uint8_t* fb_buf, int fb_len);
    int encodeLz(const uint8_t* fb_buf, int fb_len, uint8_t* buf, int len);
    int decodeLz(uint8_t* buf, int len, uint8_t* fb_buf, int fb_len);
	void output(const char* str, ...);

    virtual ZmodemFile* fetchNextFileToSend(){return nullptr;}

	char bufferM[1024*64];
	unsigned bufferLenM;
	unsigned decodeIndexM;

    std::map<uint32_t, ZmodemFile*> zmodemFileMapM;
    min_heap_item_t* inputTimerM;

	bool sendByeOnResetM;
    uint8_t peerVersionM;
};

#endif /* OSC_SESSION_H */
