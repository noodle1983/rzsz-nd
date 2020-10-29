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
#include <string.h>

struct frame_tag;
typedef struct frame_tag frame_t;

class ZmodemSession: public Fsm::Session
{
public:
	enum MyState
	{
		IDLE_STATE = 1,
		CHK_FRAME_TYPE_STATE,
		PARSE_HEX_STATE,
		PARSE_BIN_STATE,
		PARSE_BIN32_STATE,
		HANDLE_FRAME_STATE,
		WAIT_DATA_STATE,
		FILE_SELECTED_STATE,
		SEND_ZDATA_STATE,
		SEND_FLOW_CTRL_STATE,
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
		NEXT_EVT,
		CHECK_FRAME_TYPE_EVT,
	};
	typedef enum{
		DECODE_ERROR  = -1,
		DECODE_DONE   = 0,
		DECODE_PARTLY = 1
	} DecodeResult;

	enum FileSelectState{FILE_SELECT_NONE = 0, FILE_SELECTED, FILE_SELECTING};

	ZmodemSession();
	virtual ~ZmodemSession();
    static Fsm::FiniteStateMachine* getZmodemFsm();
	void initState();
	int processNetworkInput(const char* const str, const int len);
	void reset();
	void destroy();


	size_t dataCrcMatched(const size_t begin);
	unsigned short decodeCrc(const int index, int& consume_len);
	unsigned long decodeCrc32(const int index, int& consume_len);
	void sendFrameHeader(unsigned char type, long pos);
	void sendFrame(frame_t& frame);
	
	void sendBin32FrameHeader(unsigned char type, long pos);
	void sendBin32Frame(frame32_t& frame);
	unsigned convert2zline(char* dest, const unsigned dest_size, 
		const char* src, const unsigned src_len);
	void send_zsda32(char *buf, size_t length, char frameend);
	void sendZdata();

	void checkIfStartRz();
	void checkFrametype();
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

	static void deleteSelf(Fsm::Session* session);

	const char* curBuffer(){return buffer_.c_str()+decodeIndex_;}
	void eatBuffer(){
		buffer_ = buffer_.substr(decodeIndex_);
		lastCheckExcaped_ = (lastCheckExcaped_ >= decodeIndex_) ? (lastCheckExcaped_ - decodeIndex_) : 0;
		lastCheckExcapedSaved_ = (lastCheckExcapedSaved_ >= decodeIndex_) ? (lastCheckExcapedSaved_ - decodeIndex_) : 0;
		decodeIndex_=0;
	}

	bool isDoingRz(){return (getCurState().getId() !=  IDLE_STATE) || bufferParsed_;};
	int lengthToBeDecode(){return buffer_.length() - decodeIndex_;};
	const char* bufferToBeDecode(){return buffer_.c_str() + decodeIndex_;}

	template<typename ReturnStruct>
	bool decodeEscapeStruct(const int index, int& consume_len, ReturnStruct& ret)
	{
		consume_len = 0;
		char crc_buffer[sizeof(ReturnStruct)] = {0};
		unsigned i, j;
		for (i = index, j = 0; j < sizeof(ReturnStruct) && i < buffer_.length(); i++, j++){
			if (buffer_[i] == ZDLE){
				if (i + 1 < buffer_.length()){
					crc_buffer[j] = buffer_[i+1] ^ 0x40;
					i++;
					consume_len += 2;
				}else{
					break;
				}
			}else{
				crc_buffer[j] = buffer_[i];
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
private:
	void output(const char* str, ...);
	bool isToDelete(){return isDestroyed_;}
	void setDelete(){isDestroyed_ = true;}
	
	frame_t* inputFrame_;
	std::string buffer_;
	unsigned decodeIndex_;
	unsigned lastCheckExcaped_;
	unsigned lastCheckExcapedSaved_;
	unsigned long dataCrc_;
	int recv_len_;
	bool lastEscaped_;
	bool bufferParsed_;
	bool isDestroyed_;

	bool sendFinOnReset_;
	bool isSz_;
	unsigned char zsendline_tab[256];
	FileSelectState file_select_state_;

    ZmodemFile* zmodemFile_;

	uint64_t tick_;
};

#endif /* ZMODEM_SESSION_H */
