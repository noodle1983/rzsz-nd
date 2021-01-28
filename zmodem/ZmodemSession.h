#ifndef ZMODEM_SESSION_H
#define ZMODEM_SESSION_H

#include "Fsm.h"
#include "Action.h"
#include "State.h"
#include "Session.h"
#include "zmodem.h"
#include "ZmodemFile.h"
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <vector>
#include <string.h>

struct frame_tag;
typedef struct frame_tag frame_t;

class ZmodemSession: public nd::Session
{
public:
	enum MyState
	{
		IDLE_STATE = 1,
		CHK_FRAME_TYPE_STATE,
        HANDLE_ZFILE_RSP_STATE,
		WAIT_DATA_STATE,
		FILE_SELECTED_STATE,
		SEND_ZDATA_STATE,
		WAIT_ZDATA_STATE,
		HANDLE_ZDATA_STATE,
		ZEOF_STATE,
		FILE_COMPLETED_STATE,
		SEND_FLOW_CTRL_STATE,
        SEND_ZRQINIT_STATE,
        SEND_ZRINIT_STATE,
		SEND_ZFILE_STATE,
        HANDLE_ZFILE_STATE,
        CHECK_POS_STATE,
		DESTROY_STATE,
		END_STATE
	};
	enum MyEvent
	{
		NETWORK_INPUT_EVT = 0,
		RESET_EVT,
		PARSE_HEX_EVT,
		PARSE_BIN_EVT,
		PARSE_BIN32_EVT,
		HANDLE_FRAME_EVT,
		WAIT_DATA_EVT,
		FILE_SELECTED_EVT,
		SEND_ZDATA_EVT,
		SEND_ZDATA_LATER_EVT,
		DESTROY_EVT,
		SKIP_EVT,
		CHECK_FRAME_TYPE_EVT,
		SEND_FILE_EVT,
	};
	typedef enum{
		DECODE_ERROR  = -1,
		DECODE_DONE   = 0,
		DECODE_PARTLY = 1
	} DecodeResult;

	enum FileSelectState{FILE_SELECT_NONE = 0, FILE_SELECTED, FILE_SELECTING};

	ZmodemSession(nd::FiniteStateMachine* fsm);
	virtual ~ZmodemSession();
	void initState();
    void startInputTimer();
    void stopInputTimer();
    static void onInputTimerout(void *arg);
	int processNetworkInput(const char* const str, const int len);
	void reset();
	void destroy();

	size_t dataCrcMatched(const size_t begin);
	uint16_t decodeCrc(const int index, int& consume_len);
	uint32_t decodeCrc32(const int index, int& consume_len);
	void sendFrameHeader(unsigned char type, long pos);
	void sendFrame(frame_t& frame);
	
	void sendBin32FrameHeader(unsigned char type, long pos);
	void sendBin32Frame(frame32_t& frame);
	unsigned convert2zline(char* dest, const unsigned dest_size, 
		const char* src, const unsigned src_len);
	void send_zsda32(char *buf, size_t length, char frameend);
	void sendZdata();

	void checkIfStartRz();
	void parseFrame();
	void parseHexFrame();
	void parseBinFrame();
	void parseBin32Frame();
	void handleFrame();
	void sendZrinit();
	void handleZfile();
	void handleZdata();
	void handleFlowCntl();
	void sendZrpos();
	void onSentTimeout();
	void sendFileInfo();
	void sendZfin();

	static void deleteSelf(nd::Session* session);
	static void sendOO(nd::Session* session);

	const char* curBuffer(){return bufferM.c_str()+decodeIndexM;}
	void eatBuffer(){
		bufferM = bufferM.substr(decodeIndexM);
		lastCheckExcapedM = (lastCheckExcapedM >= decodeIndexM) ? (lastCheckExcapedM - decodeIndexM) : 0;
		lastCheckExcapedSavedM = (lastCheckExcapedSavedM >= decodeIndexM) ? (lastCheckExcapedSavedM - decodeIndexM) : 0;
		decodeIndexM=0;
	}

	int lengthToBeDecode(){return bufferM.length() - decodeIndexM;};
	const char* bufferToBeDecode(){return bufferM.c_str() + decodeIndexM;}

	template<typename ReturnStruct>
	bool decodeEscapeStruct(const int index, int& consume_len, ReturnStruct& ret)
	{
		consume_len = 0;
		char crc_buffer[sizeof(ReturnStruct)] = {0};
		unsigned i, j;
		for (i = index, j = 0; j < sizeof(ReturnStruct) && i < bufferM.length(); i++, j++){
			if (bufferM[i] == ZDLE){
				if (i + 1 < bufferM.length()){
					crc_buffer[j] = bufferM[i+1] ^ 0x40;
					i++;
					consume_len += 2;
				}else{
					break;
				}
			}else{
				crc_buffer[j] = bufferM[i];
				consume_len ++;
			}
		}
		if (j < sizeof(ReturnStruct)){
			consume_len = 0;
			return false;
		}

		memcpy(&ret, crc_buffer, sizeof(ReturnStruct));
		return true;
	}
protected:
	void output(const char* str, ...);
    int parseZdata();
	
	frame_t* inputFrameM;
	std::string bufferM;
	unsigned decodeIndexM;
	unsigned lastCheckExcapedM;
	unsigned lastCheckExcapedSavedM;
	uint32_t dataCrcM;
	int recvLenM;
	bool lastEscapedM;

	bool sendFinOnResetM;
	bool isSzM;

    ZmodemFile* zmodemFileM;
    min_heap_item_t* inputTimerM;

    int versionM;
};

#endif /* ZMODEM_SESSION_H */
