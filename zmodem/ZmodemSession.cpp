/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#include "ZmodemSession.h"
#include "crctab.h"
#include "zmodem.h"
#include "Version.h"
#include "stdinput.h"
#include "stdoutput.h"
#include "string.h"
#include <assert.h>

//-----------------------------------------------------------------------------

ZmodemSession::ZmodemSession(nd::FiniteStateMachine* fsm)
	: nd::Session(fsm, 0)
    , zmodemFileM(NULL)
    , inputTimerM(NULL)
    , peerVersionM(0)
    , plainDataFileM("plainZdata.data", 16)
    , encodedDataFileM("encodedZdata.data")
{
	memset(&inputFrameM, 0, sizeof(inputFrameM));
	sendFinOnResetM = false;
	isSzM = true;

}

//-----------------------------------------------------------------------------

ZmodemSession::~ZmodemSession()
{
    delete zmodemFileM;
    zmodemFileM = NULL;

    stopInputTimer();

    std::cout << reportM.str();
}

//-----------------------------------------------------------------------------

void ZmodemSession::initState()
{
	if (/*zmodemFileM ||*/ sendFinOnResetM){
		delete zmodemFileM;
		zmodemFileM = NULL;
		if ((!isSzM) && inputFrameM.type == ZFIN){
			g_stdout->sendData("OO", 2);
		}else{
			frame_t frame;
			memset(&frame, 0, sizeof(frame_t));
			frame.type = ZFIN;
			sendFrame(frame);
		}

		//LOG_SE_ERROR("");
		if (!isToDelete()) asynHandleEvent(RESET_EVT);
	}
    bufferLenM = 0;
    memset(bufferM, 0, sizeof(bufferM));
	decodeIndexM = 0;
	lastCheckExcapedM = 0;
	lastCheckExcapedSavedM = 0;
	dataCrcM = 0xFFFFFFFFL;
	sendFinOnResetM = false;
	//uploadFilePath_.clear();
	return;
}

//-----------------------------------------------------------------------------

void ZmodemSession::parseFrame()
{
    if (inputFrameM.type == ZFIN && decodeIndexM + 2 >= bufferLenM
            && bufferM[decodeIndexM] == 'O' && bufferM[decodeIndexM+1] == 'O'){
        asynHandleEvent(DESTROY_EVT);
        return;
    }

	for (; decodeIndexM < bufferLenM 
		&& ZPAD != bufferM[decodeIndexM] ; decodeIndexM ++);
	for (; decodeIndexM < bufferLenM 
		&& ZPAD == bufferM[decodeIndexM] ; decodeIndexM ++);

	if (decodeIndexM + 2 >= bufferLenM){
		return;
	}

	if (ZDLE != bufferM[decodeIndexM++]){
		handleEvent(RESET_EVT);
		return;
	}

	int frametype = bufferM[decodeIndexM++];
	if (ZHEX == frametype){
        parseHexFrame();
	}else if (ZBIN == frametype){
        parseBinFrame();
	}else if (ZBIN32 == frametype){
        parseBin32Frame();
	}else if (ZBIN64 == frametype){
        parseBin64Frame();
	}else{
		//LOG_SE_ERROR("only support(HEX,BIN,BIN32) frame");
		handleEvent(RESET_EVT);
	}
    return;
}

//-----------------------------------------------------------------------------

void ZmodemSession::parseHexFrame()
{
	if (decodeIndexM + sizeof(hex_t) >= bufferLenM)
		return;
	hex_t  hexframe;
	memcpy(&hexframe, curBuffer(), sizeof(hex_t));
	decodeIndexM += sizeof(hex_t);

	frame_t frame;
    convHex2Plain(&hexframe, &frame);
    if (frame.crc != calcFrameCrc(&frame)){
		LOG_SE_ERROR("crc error!");
        handleEvent(RESET_EVT);
        return ;
    }

	unsigned old_index = decodeIndexM;
	for (; decodeIndexM < bufferLenM 
		&& ('\r' == bufferM[decodeIndexM] 
			|| '\n' == bufferM[decodeIndexM]
			|| -118 == bufferM[decodeIndexM]) ; decodeIndexM ++);
	if (old_index == decodeIndexM){
		LOG_SE_ERROR("no line seed found!");
        handleEvent(RESET_EVT);
        return ;
    }
	if (frame.type != ZACK && frame.type != ZFIN && bufferM[decodeIndexM++] != XON){
		LOG_SE_ERROR("XON expected!");
        handleEvent(RESET_EVT);
        return ;
    }
	eatBuffer();
	memset(&inputFrameM, 0, sizeof(inputFrameM));
	memcpy(&inputFrameM, &frame, 5);
    LOG_INFO(getSessionName() 
        << "[" << getSessionId() << "] " << getCurState().getName() << " "
        << "got Hex frame:" << getTypeStr(inputFrameM.type));
	handleEvent(HANDLE_FRAME_EVT);
	return;
}

//-----------------------------------------------------------------------------

void ZmodemSession::parseBinFrame()
{
	if (decodeIndexM + sizeof(frame_t) >= bufferLenM)
		return;
	frame_t frame;
	int frame_len = 0;
    if (!decodeEscapeStruct<frame_t>(decodeIndexM, frame_len, frame)){
		return;
	}
	decodeIndexM += frame_len;

    if (frame.crc != calcFrameCrc(&frame)){
		LOG_SE_ERROR("bin crc error!");
        handleEvent(RESET_EVT);
        return ;
    }
	eatBuffer();
	memset(&inputFrameM, 0, sizeof(inputFrameM));
	memcpy(&inputFrameM, &frame, 5);
    LOG_INFO(getSessionName() 
        << "[" << getSessionId() << "] " << getCurState().getName() << " "
        << "got Bin frame:" << getTypeStr(inputFrameM.type));
	handleEvent(HANDLE_FRAME_EVT);
	return;
}

//-----------------------------------------------------------------------------

void ZmodemSession::parseBin32Frame()
{
	if (decodeIndexM + sizeof(frame32_t) > bufferLenM)
		return;
	frame32_t frame;
	int frame_len = 0;
	if (!decodeEscapeStruct<frame32_t>(decodeIndexM, frame_len, frame)){
		return;
	}
	decodeIndexM += frame_len;

    if (frame.crc != calcFrameCrc32(&frame)){
		LOG_SE_ERROR("bin32 crc error!");
        handleEvent(RESET_EVT);
        return ;
    }
	eatBuffer();
	memset(&inputFrameM, 0, sizeof(inputFrameM));
	memcpy(&inputFrameM, &frame, 5);
    LOG_INFO(getSessionName() 
        << "[" << getSessionId() << "] " << getCurState().getName() << " "
        << "got Bin32 frame:" << getTypeStr(inputFrameM.type));
	handleEvent(HANDLE_FRAME_EVT);
	return;
}

//-----------------------------------------------------------------------------

void ZmodemSession::parseBin64Frame()
{
	if (decodeIndexM + sizeof(frame64_t) > bufferLenM)
		return;
	frame64_t frame;
	int frame_len = 0;
	if (!decodeEscapeStruct<frame64_t>(decodeIndexM, frame_len, frame)){
		return;
	}
	decodeIndexM += frame_len;

    if (frame.crc != calcFrame64Crc32(&frame)){
		LOG_SE_ERROR("bin32 crc error!");
        handleEvent(RESET_EVT);
        return ;
    }
	eatBuffer();
	memset(&inputFrameM, 0, sizeof(inputFrameM));
	memcpy(&inputFrameM, &frame, 9);
    LOG_INFO(getSessionName() 
        << "[" << getSessionId() << "] " << getCurState().getName() << " "
        << "got Bin64 frame:" << getTypeStr(inputFrameM.type));
	handleEvent(HANDLE_FRAME_EVT);
	return;
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendZdata()
{
	const unsigned BUFFER_LEN = 1024;
	char buffer[BUFFER_LEN + 16] = {0};

	//if (frontend_->send_bufferMsize() > 1024*1024){
	//	if (!isToDelete()) asynHandleEvent(SEND_ZDATA_LATER_EVT);
	//	return;
	//}

	unsigned len = zmodemFileM->read(buffer, BUFFER_LEN);
	char frameend = zmodemFileM->isGood() ? ZCRCG : ZCRCE;
	send_zsda32(buffer, len, frameend);
		
	if(!zmodemFileM->isGood()){
		checkSendFrameHeader(ZEOF, zmodemFileM->getPos());
		return;
	}else{
		if (!isToDelete()) asynHandleEvent(SEND_ZDATA_EVT);
		return;
	}
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendFrame(frame_t& frame)
{
    frame.crc = calcFrameCrc(&frame);
    hex_t hexframe;
    convPlain2Hex(&frame, &hexframe);

    char buf[32] = {0};
    int len = 0;
    memcpy(buf+len, HEX_PREFIX, 4);
    len += 4;
    memcpy(buf+len, &hexframe, sizeof (hex_t));
    len += sizeof (hex_t);
	buf[len++] = '\r';
	buf[len++] = 0212;
	if (frame.type != ZFIN && frame.type != ZACK){
		buf[len++] = XON;
	}
    g_stdout->sendData(buf, len);
    LOG_SE_INFO("sent Hex frame:" << getTypeStr(frame.type));
}

//-----------------------------------------------------------------------------

void ZmodemSession::checkSendFrameHeader(unsigned char type, uint64_t pos)
{
   if (pos >= 0x100000000) {
       sendBin64FrameHeader(type, pos);
   }
   else{
       sendBin32FrameHeader(type, (uint32_t)pos);
   }
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendFrameHeader(unsigned char type, uint32_t pos)
{
	frame_t frame;
    memset(&frame, 0, sizeof(frame_t));
    frame.type = type;
	frame.flag[ZP0] = (unsigned char)(pos);
	frame.flag[ZP1] = (unsigned char)(pos>>8);
	frame.flag[ZP2] = (unsigned char)(pos>>16);
	frame.flag[ZP3] = (unsigned char)(pos>>24);
    sendFrame(frame);
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendBin32FrameHeader(unsigned char type, uint32_t pos)
{
	frame32_t frame;
    memset(&frame, 0, sizeof(frame32_t));
    frame.type = type;
	frame.flag[ZP0] = pos;
	frame.flag[ZP1] = pos>>8;
	frame.flag[ZP2] = pos>>16;
	frame.flag[ZP3] = pos>>24;
    sendBin32Frame(frame);
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendBin64FrameHeader(unsigned char type, uint64_t pos)
{
	frame64_t frame;
    memset(&frame, 0, sizeof(frame64_t));
    frame.type = type;
	frame.flag[ZP0] = pos;
	frame.flag[ZP1] = pos>>8;
	frame.flag[ZP2] = pos>>16;
	frame.flag[ZP3] = pos>>24;
	frame.flag[ZP4] = pos>>32;
	frame.flag[ZP5] = pos>>40;
	frame.flag[ZP6] = pos>>48;
	frame.flag[ZP7] = pos>>56;
    sendBin64Frame(frame);
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendBin32Frame(frame32_t& frame)
{
    frame.crc = calcFrameCrc32(&frame);

    char buf[256] = {0};
    int len = 0;
    memcpy(buf+len, BIN32_PREFIX, 4);
    len += 4;

    len += convert2zline(buf+len, sizeof(buf) -len, (char*)&frame, sizeof(frame));
    g_stdout->sendData(buf, len);
    LOG_SE_INFO("sent Bin32 frame:" << getTypeStr(frame.type));
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendBin64Frame(frame64_t& frame)
{
    frame.crc = calcFrame64Crc32(&frame);

    char buf[256] = {0};
    int len = 0;
    memcpy(buf+len, BIN64_PREFIX, 4);
    len += 4;

    len += convert2zline(buf+len, sizeof(buf) -len, (char*)&frame, sizeof(frame));
    g_stdout->sendData(buf, len);
    LOG_SE_INFO("sent Bin32 frame:" << getTypeStr(frame.type));
}


//-----------------------------------------------------------------------------

unsigned ZmodemSession::convert2zline(char* dest, const unsigned dest_size, 
		const char* src, const unsigned src_len)
{
	char lastsent = 0;
	unsigned ret_len = 0;
	for (unsigned i = 0; i < src_len && ret_len < dest_size; i++){
		char c = src[i];
		unsigned char escape_value = (zsendline_tab[(unsigned char) (c&=0377)]);
		if (0 ==  escape_value){
			dest[ret_len++] = (lastsent = c); 
		}else if (1 ==  escape_value){
			dest[ret_len++] = ZDLE;
			c ^= 0100;
			dest[ret_len++] = (lastsent = c);
		}else if (2 ==  escape_value){
			if ((lastsent & 0177) != '@') {
				dest[ret_len++] = (lastsent = c);
			} else {
				dest[ret_len++] = (ZDLE);
				c ^= 0100;
				dest[ret_len++] = (lastsent = c);
			}
		}

	}
	return ret_len;

}

//-----------------------------------------------------------------------------

int ZmodemSession::parseZdata()
{
	//curBuffer() with len bufferLenM - decodeIndexM
	//offset in inputFrameM

	for (; lastCheckExcapedM + 1 < bufferLenM; lastCheckExcapedM++, lastCheckExcapedSavedM++){
		if (bufferM[lastCheckExcapedM] == ZDLE){
			if (lastCheckExcapedM + 6 > bufferLenM){
                // wait more data
				return 0;
			}
			if (ZCRCE == bufferM[lastCheckExcapedM + 1]){
				uint32_t calc_crc = ~UPDC32(bufferM[lastCheckExcapedM+1], dataCrcM);
				int consume_len = 0;
				uint32_t recv_crc = decodeCrc32(lastCheckExcapedM+2, consume_len);
				if (consume_len == 0){
                    // wait more data
					return 0;
				}

				if (calc_crc == recv_crc){
					lastCheckExcapedM += 2 + consume_len;
					dataCrcM = 0xFFFFFFFFL;
					
					return ZCRCE;
				}
                else{
                    LOG_SE_ERROR("ZCRCE crc error!");
                    return -1;
                }
			}else if (ZCRCG == bufferM[lastCheckExcapedM + 1]){	
				uint32_t calc_crc = ~UPDC32((unsigned char)(bufferM[lastCheckExcapedM+1]), dataCrcM);
				int consume_len = 0;
				uint32_t recv_crc = decodeCrc32(lastCheckExcapedM+2, consume_len);
				if (consume_len == 0){
                    // wait more data
					return 0;
				}

				if (calc_crc == recv_crc){
					assert(lastCheckExcapedSavedM- decodeIndexM == 1024);
					lastCheckExcapedM += 2 + consume_len;
					dataCrcM = 0xFFFFFFFFL;

					return ZCRCG;
				}else {
                    return -1;
				}
				//else it is normal char

			}else if (ZCRCQ == bufferM[lastCheckExcapedM + 1]){
				uint32_t calc_crc = ~UPDC32(bufferM[lastCheckExcapedM+1], dataCrcM);
				int consume_len = 0;
				uint32_t recv_crc = decodeCrc32(lastCheckExcapedM+2, consume_len);
				if (consume_len == 0){
                    // wait more date
					return 0;
				}

				if (calc_crc == recv_crc){
					lastCheckExcapedM += 2 + consume_len;
					dataCrcM = 0xFFFFFFFFL;
					checkSendFrameHeader(ZACK, zmodemFileM->getPos());
					return ZCRCQ;
				}else {
                    LOG_SE_ERROR("ZCRCQ crc error!");
                    return -1;
				}

			}else if (ZCRCW == bufferM[lastCheckExcapedM + 1]){
				uint32_t calc_crc = ~UPDC32(bufferM[lastCheckExcapedM+1], dataCrcM);
				int consume_len = 0;
				uint32_t recv_crc = decodeCrc32(lastCheckExcapedM+2, consume_len);
				if (consume_len == 0){
                    //wait more data
					return 0;
				}

				if (calc_crc == recv_crc){
					lastCheckExcapedM += 2 + consume_len;
					dataCrcM = 0xFFFFFFFFL;
					return ZCRCW;
				}else {
                    LOG_SE_ERROR("ZCRCW crc error!");
                    return -1;
				}
			}else{
				lastCheckExcapedM++;
				bufferM[lastCheckExcapedSavedM] = bufferM[lastCheckExcapedM] ^ 0x40;
				dataCrcM = UPDC32((unsigned char)(bufferM[lastCheckExcapedSavedM]), dataCrcM);
			}
		}else{
			bufferM[lastCheckExcapedSavedM] = bufferM[lastCheckExcapedM] ;
			dataCrcM = UPDC32((unsigned char)(bufferM[lastCheckExcapedSavedM]), dataCrcM);
		}
	}
	return 0;
}
//-----------------------------------------------------------------------------

int ZmodemSession::parseZdataString()
{
    if (bufferLenM - decodeIndexM > 1024){
        asynHandleEvent(DESTROY_EVT);
        return -1;
    }
    int ret = parseZdata();
    if (ret == 0) {return 0;}
    if (ret < 0) {
        asynHandleEvent(DESTROY_EVT);
        return ret;
    }

    int nullCount = 0;
    for(unsigned i = decodeIndexM; i < bufferLenM; i++){
        if (bufferM[i] == 0){nullCount++;}
    }
    return nullCount;
}

//-----------------------------------------------------------------------------

void ZmodemSession::parseZdataStringDone(){
	decodeIndexM = lastCheckExcapedM;//crc len
	if (*curBuffer() == XON){
		decodeIndexM++;
	}

	eatBuffer();
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendZrinit()
{
	frame_t frame;
    memset(&frame, 0, sizeof(frame_t));
    frame.type = ZRINIT;
    frame.flag[ZF0] = CANFC32|CANFDX|CANOVIO;
    frame.flag[ZF1] = g_options->rzDirModeM ? ZF1_RZ_DIR : 0;
    frame.flag[ZF3] = ZVERSION;
	sendFrame(frame);
}

//-----------------------------------------------------------------------------

void ZmodemSession::handleZfile()
{
    if (inputFrameM.type == ZCOMPL){
        asynHandleEvent(SKIP_EVT);
        return;
    }
	if (inputFrameM.type != ZFILE){
        asynHandleEvent(DESTROY_EVT);
        return;
    }
    peerVersionM = inputFrameM.flag[ZF3];

    int strCnt = parseZdataString();
    if (strCnt < 2) {return;}

	std::string filename(curBuffer());
	decodeIndexM += filename.length() + 1;
	std::string fileinfo(curBuffer());
	decodeIndexM += fileinfo.length() + 1;

    parseZdataStringDone();

	if (zmodemFileM)
		delete zmodemFileM;

	zmodemFileM = new ZmodemFile(g_options->getServerWorkingDir(), filename, fileinfo);
    uint32_t existCrc = 0;
	uint64_t len = zmodemFileM->getExistLen(existCrc);
	if (len > 0 && peerVersionM > 0) {
        sendZCommand(ZCMD_CHK_LAST_BREAK, "%llu %u",  (long long unsigned)len, existCrc);
        asynHandleEvent(NEXT_EVT);
		return;
	}

    zmodemFileM->openWrite(false);
	checkSendFrameHeader(ZRPOS, zmodemFileM->getPos());
    asynHandleEvent(NEXT_EVT);
}

//-----------------------------------------------------------------------------

void ZmodemSession::waitZdata()
{
    if (inputFrameM.type == ZDATA){
        handleEvent(NEXT_EVT);
        return;
    }
    if (inputFrameM.type == ZFILE){
        asynHandleEvent(SKIP_EVT);
        return;
    }
    if (inputFrameM.type == ZCOMPL){
        asynHandleEvent(NETWORK_INPUT_EVT);
        return;
    }
	if (inputFrameM.type != ZACK){
        asynHandleEvent(DESTROY_EVT);
        return;
    }

    uint64_t pos = getPos(&inputFrameM);
    if (zmodemFileM != NULL && !zmodemFileM->isGood() && pos < zmodemFileM->getSize()) {
        zmodemFileM->openWrite(pos > 0);
        if (pos > 0) {
            zmodemFileM->setWritePos(pos);
        }
        checkSendFrameHeader(ZRPOS, zmodemFileM->getPos());
        return;
    }
    else if (zmodemFileM != NULL && !zmodemFileM->isGood() && pos == zmodemFileM->getSize()) {
        checkSendFrameHeader(ZSKIP, 0);
        return;
    }

    zmodemFileM->openWrite(false);
	checkSendFrameHeader(ZRPOS, zmodemFileM->getPos());
}

//-----------------------------------------------------------------------------

void ZmodemSession::send_zsda32(char *buf, size_t length, char frameend)
{
	char send_buf[2048+128];
	size_t send_len = 0;
	uint32_t crc;

	send_len = convert2zline(send_buf, sizeof(send_buf), buf, length);
	send_buf[send_len++] = ZDLE;
	send_buf[send_len++] = frameend;

	//crc includes the frameend
	buf[length] = frameend;
	crc = calcBufferCrc32(buf, length+1);
	send_len +=convert2zline(send_buf+send_len, sizeof(send_buf) - send_len, (char*)&crc, sizeof(crc)); 
	if (frameend == ZCRCW) {
		send_buf[send_len++] = (XON);  
	}
	g_stdout->sendData(send_buf, send_len);
}

//-----------------------------------------------------------------------------

uint16_t ZmodemSession::decodeCrc(const int index, int& consume_len)
{
	uint16_t ret = 0;
	decodeEscapeStruct<uint16_t>(index, consume_len, ret);
	return ret;
}

//-----------------------------------------------------------------------------

uint32_t ZmodemSession::decodeCrc32(const int index, int& consume_len)
{
	uint32_t ret = 0;
	decodeEscapeStruct<uint32_t>(index, consume_len, ret);
	return ret;
}

//-----------------------------------------------------------------------------

void ZmodemSession::handleZdata()
{
	if (inputFrameM.type != ZDATA){
        asynHandleEvent(DESTROY_EVT);
        return;
    }
	//curBuffer() with len bufferLenM - decodeIndexM
	//offset in inputFrameM

	for (; lastCheckExcapedM + 1 < bufferLenM; lastCheckExcapedM++, lastCheckExcapedSavedM++){
		if (bufferM[lastCheckExcapedM] == ZDLE){
			if (lastCheckExcapedM + 6 > bufferLenM){
                // wait more data
				return;
			}
			if (ZCRCE == bufferM[lastCheckExcapedM + 1]){
				uint32_t calc_crc = ~UPDC32(bufferM[lastCheckExcapedM+1], dataCrcM);
				int consume_len = 0;
				uint32_t recv_crc = decodeCrc32(lastCheckExcapedM+2, consume_len);
				if (consume_len == 0){
                    // wait more data
					return;
				}

				if (calc_crc == recv_crc){
					lastCheckExcapedM += 1 + consume_len;
                    if (g_options->shouldLogTestData()){
                        plainDataFileM.write(curBuffer(), lastCheckExcapedSavedM - decodeIndexM);
                        plainDataFileM.close();
                        encodedDataFileM.write(bufferM + lastCheckExcapedM, consume_len + 2);
                        encodedDataFileM.close();
                    }
					if (!zmodemFileM->write(curBuffer(), lastCheckExcapedSavedM - decodeIndexM)){
						handleEvent(RESET_EVT);
						return;
					}
					lastCheckExcapedSavedM = lastCheckExcapedM;
					decodeIndexM = lastCheckExcapedM+1;
					dataCrcM = 0xFFFFFFFFL;
					
                    eatBuffer();
					handleEvent(NEXT_EVT);
					return;
				} else {
                    if (g_options->shouldLogTestData()){
                        plainDataFileM.write(curBuffer(), lastCheckExcapedSavedM - decodeIndexM);
                        plainDataFileM.close();
                        encodedDataFileM.write(bufferM + lastCheckExcapedM, consume_len + 2);
                        encodedDataFileM.close();
                    }
                    LOG_SE_ERROR("ZCRCE crc error! at len:" << (lastCheckExcapedSavedM - decodeIndexM)
                        << ", recv:" << recv_crc << ", calc:" << calc_crc 
                        << ", valided pos:" << zmodemFileM->getPos());
					handleEvent(RESET_EVT);
					return;
				}
			}else if (ZCRCG == bufferM[lastCheckExcapedM + 1]){	
				uint32_t calc_crc = ~UPDC32((unsigned char)(bufferM[lastCheckExcapedM+1]), dataCrcM);
				int consume_len = 0;
				uint32_t recv_crc = decodeCrc32(lastCheckExcapedM+2, consume_len);
				if (consume_len == 0){
                    // wait more data
					return;
				}

				if (calc_crc == recv_crc){
					assert(lastCheckExcapedSavedM - decodeIndexM == 1024);
					lastCheckExcapedM += 1 + consume_len;
                    if (g_options->shouldLogTestData()){
                        plainDataFileM.write(curBuffer(), lastCheckExcapedSavedM - decodeIndexM);
                        plainDataFileM.close();
                        encodedDataFileM.write(bufferM + lastCheckExcapedM, consume_len + 2);
                        encodedDataFileM.close();
                    }
					if (!zmodemFileM->write(curBuffer(), lastCheckExcapedSavedM - decodeIndexM)){
						handleEvent(RESET_EVT);
						return;
					}
					lastCheckExcapedSavedM = lastCheckExcapedM;
					decodeIndexM = lastCheckExcapedM+1;
                    eatBuffer();
					dataCrcM = 0xFFFFFFFFL;
                    asynHandleEvent(NETWORK_INPUT_EVT);
                    LOG_SE_INFO("ZCRCG len:" << zmodemFileM->getPos() << "/" << zmodemFileM->getSize()
                            << ", crc:" << calc_crc);
                    return;
				} else {
                    if (g_options->shouldLogTestData()){
                        plainDataFileM.write(curBuffer(), lastCheckExcapedSavedM - decodeIndexM);
                        plainDataFileM.close();
                        encodedDataFileM.write(bufferM + lastCheckExcapedM, consume_len + 2);
                        encodedDataFileM.close();
                    }
                    LOG_SE_ERROR("ZCRCG crc error! at len:" << (lastCheckExcapedSavedM - decodeIndexM)
                        << ", recv:" << recv_crc << ", calc:" << calc_crc 
                        << ", valided pos:" << zmodemFileM->getPos());
					handleEvent(RESET_EVT);
					return;
				}
				//else it is normal char

			}else if (ZCRCQ == bufferM[lastCheckExcapedM + 1]){
				uint32_t calc_crc = ~UPDC32(bufferM[lastCheckExcapedM+1], dataCrcM);
				int consume_len = 0;
				uint32_t recv_crc = decodeCrc32(lastCheckExcapedM+2, consume_len);
				if (consume_len == 0){
                    // wait more date
					return;
				}

				if (calc_crc == recv_crc){
					lastCheckExcapedM += 1 + consume_len;
                    if (g_options->shouldLogTestData()){
                        plainDataFileM.write(curBuffer(), lastCheckExcapedSavedM - decodeIndexM);
                        plainDataFileM.close();
                        encodedDataFileM.write(bufferM + lastCheckExcapedM, consume_len + 2);
                        encodedDataFileM.close();
                    }
					if (!zmodemFileM->write(curBuffer(), lastCheckExcapedSavedM - decodeIndexM)){
						handleEvent(RESET_EVT);
						return;
					}
					lastCheckExcapedSavedM = lastCheckExcapedM;
					decodeIndexM = lastCheckExcapedM+1;
					dataCrcM = 0xFFFFFFFFL;
                    eatBuffer();
					checkSendFrameHeader(ZACK, zmodemFileM->getPos());
					continue;
				} else {
                    if (g_options->shouldLogTestData()){
                        plainDataFileM.write(curBuffer(), lastCheckExcapedSavedM - decodeIndexM);
                        plainDataFileM.close();
                        encodedDataFileM.write(bufferM + lastCheckExcapedM, consume_len + 2);
                        encodedDataFileM.close();
                    }
                    LOG_SE_ERROR("ZCRCQ crc error! at len:" << (lastCheckExcapedSavedM - decodeIndexM)
                        << ", recv:" << recv_crc << ", calc:" << calc_crc 
                        << ", valided pos:" << zmodemFileM->getPos());
					handleEvent(RESET_EVT);
					return;
				}

			}else if (ZCRCW == bufferM[lastCheckExcapedM + 1]){
				uint32_t calc_crc = ~UPDC32(bufferM[lastCheckExcapedM+1], dataCrcM);
				int consume_len = 0;
				uint32_t recv_crc = decodeCrc32(lastCheckExcapedM+2, consume_len);
				if (consume_len == 0){
                    //wait more data
					return;
				}

				if (calc_crc == recv_crc){
					lastCheckExcapedM += 1 + consume_len;
                    if (g_options->shouldLogTestData()){
                        plainDataFileM.write(curBuffer(), lastCheckExcapedSavedM - decodeIndexM);
                        plainDataFileM.close();
                        encodedDataFileM.write(bufferM + lastCheckExcapedM, consume_len + 2);
                        encodedDataFileM.close();
                    }
					if (!zmodemFileM->write(curBuffer(), lastCheckExcapedSavedM - decodeIndexM)){
						handleEvent(RESET_EVT);
						return;
					}
					lastCheckExcapedSavedM = lastCheckExcapedM;
					decodeIndexM = lastCheckExcapedM+1;
					dataCrcM = 0xFFFFFFFFL;
                    eatBuffer();
					checkSendFrameHeader(ZACK, zmodemFileM->getPos());
					continue;
				} else {
                    if (g_options->shouldLogTestData()){
                        plainDataFileM.write(curBuffer(), lastCheckExcapedSavedM - decodeIndexM);
                        plainDataFileM.close();
                        encodedDataFileM.write(bufferM + lastCheckExcapedM, consume_len + 2);
                        encodedDataFileM.close();
                    }
                    LOG_SE_ERROR("ZCRCW crc error! at len:" << (lastCheckExcapedSavedM - decodeIndexM)
                        << ", recv:" << recv_crc << ", calc:" << calc_crc 
                        << ", valided pos:" << zmodemFileM->getPos());
					handleEvent(RESET_EVT);
					return;
				}
			}else{
                if (g_options->shouldLogTestData()){
                    encodedDataFileM.write(bufferM + lastCheckExcapedM, 2);
                    uint64_t offset = lastCheckExcapedSavedM - decodeIndexM + 1;
                    if (offset > 0 && (offset % 16 == 0)){ encodedDataFileM.switchLine(); }
                }
				lastCheckExcapedM++;
				bufferM[lastCheckExcapedSavedM] = bufferM[lastCheckExcapedM] ^ 0x40;
				dataCrcM = UPDC32((unsigned char)(bufferM[lastCheckExcapedSavedM]), dataCrcM);
			}
		}else{
            if (g_options->shouldLogTestData()){
                encodedDataFileM.write(bufferM + lastCheckExcapedM, 1);
                uint64_t offset = lastCheckExcapedSavedM - decodeIndexM + 1;
                if (offset > 0 && (offset % 16 == 0)){ encodedDataFileM.switchLine(); }
            }
			bufferM[lastCheckExcapedSavedM] = bufferM[lastCheckExcapedM];
			dataCrcM = UPDC32((unsigned char)(bufferM[lastCheckExcapedSavedM]), dataCrcM);
		}
	}
	eatBuffer();
    // wait more data
	return;
}

//-----------------------------------------------------------------------------

void ZmodemSession::onInputTimerout(void *arg)
{
    ZmodemSession* self = (ZmodemSession*)arg;
    char buffer[1024] = {0};

    if (sizeof(self->bufferM) - self->bufferLenM < sizeof(buffer)){
        self->processNetworkInput(buffer, 0);
        self->inputTimerM = NULL;
        self->startInputTimer();
        return;
    }
    
    int len = 0;
    while((sizeof(self->bufferM) - self->bufferLenM > sizeof(buffer)) && (len = g_stdin->getInput(buffer, sizeof(buffer))) > 0)
    {
        self->processNetworkInput(buffer, len);
    }
    self->inputTimerM = NULL;
    self->startInputTimer();
}

void ZmodemSession::startInputTimer()
{
    if (inputTimerM){stopInputTimer();}
    inputTimerM = g_processor->addLocalTimer(10, onInputTimerout, this);
}

//-----------------------------------------------------------------------------

void ZmodemSession::stopInputTimer()
{
    if (!inputTimerM){return;}
    g_processor->cancelLocalTimer(inputTimerM);
}

//-----------------------------------------------------------------------------

int ZmodemSession::processNetworkInput(const char* const str, const int len)
{	
    if (len > 0){
        memcpy(bufferM + bufferLenM, str, len);
        bufferLenM += len;
    }
	asynHandleEvent(NETWORK_INPUT_EVT);
	return 0;
}

//-----------------------------------------------------------------------------

void ZmodemSession::reset()
{
	const char canistr[] =
	{
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0
	};
	g_stdout->sendData(canistr, strlen(canistr));
	handleEvent(RESET_EVT);
}

//-----------------------------------------------------------------------------

void ZmodemSession::output(const char* str, ...)
{
    char buffer[1024] = { 0 };
	va_list ap;
	va_start(ap, str);
	vsnprintf(buffer, sizeof(buffer), str, ap);
	va_end(ap);

    LOG_SE_DEBUG(buffer);
}

//-----------------------------------------------------------------------------

void ZmodemSession::destroy()
{
	asynHandleEvent(DESTROY_EVT);
	setDelete();
}

//-----------------------------------------------------------------------------

void ZmodemSession::deleteSelf(nd::Session* session)
{
    resetTty();
    delete session;
    g_processor->waitStop();
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendZfin()
{
    frame_t frame;
    memset(&frame, 0, sizeof(frame_t));
    frame.type = ZFIN;
    sendFrame(frame);
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendOO()
{
	if (inputFrameM.type != ZFIN){
        asynHandleEvent(DESTROY_EVT);
        return;
    }
    const char* oo = "OO";
    g_stdout->sendData(oo, strlen(oo));
    asynHandleEvent(RESET_EVT);
}

//-----------------------------------------------------------------------------

void ZmodemSession::handleZfileRsp()
{
	if (zmodemFileM == NULL){
        asynHandleEvent(DESTROY_EVT);
        return;
    }
    if (inputFrameM.type == ZSKIP){
        asynHandleEvent(SKIP_EVT);
        return;
    }

    // validate existing file content
    if (inputFrameM.type == ZCOMMAND && inputFrameM.flag[ZF0] == ZCMD_CHK_LAST_BREAK){
        int strCnt = parseZdataString();
        if (strCnt < 1) {return;}
        uint64_t validLen = zmodemFileM->validateFileCrc(bufferM + decodeIndexM);
        parseZdataStringDone();

        checkSendFrameHeader(ZACK, validLen);
        return;
    }

    // client requires data at pos
	if (inputFrameM.type != ZRPOS){
        asynHandleEvent(DESTROY_EVT);
        return;
    }

    uint64_t pos = getPos(&inputFrameM);
    LOG_INFO(getSessionName() 
        << "[" << getSessionId() << "] " << getCurState().getName() << " "
        << "got ZRPOS:" << pos);

    zmodemFileM->setReadPos(pos);
    checkSendFrameHeader(ZDATA, pos);
    asynHandleEvent(SEND_ZDATA_EVT);
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendZCommand(char command, const char* format, ...)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

   	va_list ap;
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer) - 1, format, ap);
	va_end(ap);


    frame32_t frame;
    memset(&frame, 0, sizeof(frame));
    frame.type = ZCOMMAND;
    frame.flag[ZF0] = command;
    sendBin32Frame(frame);
    send_zsda32(buffer, strlen(buffer) + 1, ZCRCW);
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendClientWorkingDir()
{
    auto clientWorkingDir = g_options->getClientWorkingDir();
    if(clientWorkingDir.empty()){return;}

    sendZCommand(ZCMD_SET_CLIENT_WORKDIR, "%s", clientWorkingDir.c_str());
}

