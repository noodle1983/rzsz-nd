#include "ZmodemSession.h"
#include "crctab.h"
#include "zmodem.h"
#include "stdinput.h"
#include "stdoutput.h"
#include "string.h"
#include <assert.h>

//-----------------------------------------------------------------------------

ZmodemSession::ZmodemSession(nd::FiniteStateMachine* fsm)
	: nd::Session(fsm, 0)
	, lastEscapedM(false)
    , zmodemFileM(NULL)
    , inputTimerM(NULL)
    , versionM(0)
{
	inputFrameM = new frame_t;
	sendFinOnResetM = false;
	isSzM = true;

}

//-----------------------------------------------------------------------------

ZmodemSession::~ZmodemSession()
{
    delete zmodemFileM;
    zmodemFileM = NULL;

	delete inputFrameM;
    inputFrameM = NULL;

    stopInputTimer();
}

//-----------------------------------------------------------------------------

void ZmodemSession::initState()
{
	if (/*zmodemFileM ||*/ sendFinOnResetM){
		delete zmodemFileM;
		zmodemFileM = NULL;
		if ((!isSzM) && inputFrameM->type == ZFIN){
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
	recvLenM = 0;
	lastEscapedM = false;
	sendFinOnResetM = false;
	//uploadFilePath_.clear();
	return;
}

//-----------------------------------------------------------------------------

void ZmodemSession::parseFrame()
{
    if (inputFrameM->type == ZFIN && decodeIndexM + 2 >= bufferLenM
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
        return;
	}else if (ZBIN == frametype){
        parseBinFrame();
        return;
	}else if (ZBIN32 == frametype){
        parseBin32Frame();
        return;
	}else{
		//LOG_SE_ERROR("only support(HEX,BIN,BIN32) frame");
		handleEvent(RESET_EVT);
		return;
	}
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
	memcpy(inputFrameM, &frame, sizeof(frame_t));
    LOG_INFO(getSessionName() 
        << "[" << getSessionId() << "] " << getCurState().getName() << " "
        << "got Hex frame:" << getTypeStr(inputFrameM->type));
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
	memcpy(inputFrameM, &frame, sizeof(frame_t));
    LOG_INFO(getSessionName() 
        << "[" << getSessionId() << "] " << getCurState().getName() << " "
        << "got Bin frame:" << getTypeStr(inputFrameM->type));
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
	memcpy(inputFrameM, &frame, sizeof(frame_t));
    LOG_INFO(getSessionName() 
        << "[" << getSessionId() << "] " << getCurState().getName() << " "
        << "got Bin32 frame:" << getTypeStr(inputFrameM->type));
	handleEvent(HANDLE_FRAME_EVT);
	return;
}

//-----------------------------------------------------------------------------

void ZmodemSession::handleFrame()
{
	switch (inputFrameM->type){
    case ZRQINIT:
        return sendZrinit();

    case ZFILE: 
		isSzM = true;
		return handleZfile();
    case ZDATA:
		return handleZdata();
    case ZEOF:
		if (zmodemFileM){
			delete zmodemFileM;
			zmodemFileM = NULL;
		}
		return sendZrinit();
    case ZFIN:
		sendFinOnResetM = true;
		handleEvent(RESET_EVT);
		return;
    case ZRINIT:
		isSzM = false;
		return;
	case ZRPOS: {
			if (!zmodemFileM) {
				sendFinOnResetM = true;
				sendBin32FrameHeader(ZCAN, 0);
				sendBin32FrameHeader(ZABORT, 0);
				handleEvent(RESET_EVT);
				return;
			}
			unsigned pos = getPos(inputFrameM);
			output("remote set pos to %d", pos);
			zmodemFileM->setPos(pos);
			sendBin32FrameHeader(ZDATA, zmodemFileM->getPos());
			sendZdata();
		}
		return;
    case ZNAK:
		if (decodeIndexM < bufferLenM){
			handleEvent(NETWORK_INPUT_EVT);
		}
		return;
    case ZSINIT:
		LOG_SE_ERROR("unexpected frame type:ZSINIT");
		break;
    case ZACK:
		LOG_SE_ERROR("unexpected frame type:ZACK");
		break;
    case ZSKIP:
		LOG_SE_ERROR("unexpected frame type:ZSKIP. no permission to write file?");
		break;
    case ZABORT:
		LOG_SE_ERROR("unexpected frame type:ZABORT");
		break;
    case ZFERR:
		LOG_SE_ERROR("unexpected frame type:ZFERR");
		break;
    case ZCRC:
		LOG_SE_ERROR("unexpected frame type:ZCRC");
		break;
    case ZCHALLENGE:
		LOG_SE_ERROR("unexpected frame type:ZCHALLENGE");
		break;
    case ZCOMPL:
		LOG_SE_ERROR("unexpected frame type:ZCOMPL");
		break;
    case ZCAN:
		LOG_SE_ERROR("unexpected frame type:ZCAN");
		break;
    case ZFREECNT:
		LOG_SE_ERROR("unexpected frame type:ZFREECNT");
		break;
    case ZCOMMAND:
		LOG_SE_ERROR("unexpected frame type:ZCOMMAND");
		break;
    case ZSTDERR:
		LOG_SE_ERROR("unexpected frame type:ZSTDERR");
		break;
    default:
        LOG_SE_ERROR("invalid frame type!");
        break;


    }
    handleEvent(RESET_EVT);
    return ;


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
		sendBin32FrameHeader(ZEOF, zmodemFileM->getPos());
		return;
	}else{
		if (!isToDelete()) asynHandleEvent(SEND_ZDATA_EVT);
		return;
	}
}

//-----------------------------------------------------------------------------

void ZmodemSession::onSentTimeout()
{
	if (!zmodemFileM->isGood()) {
		//LOG_SE_ERROR(".");
		newTimer(1000);
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
}
//-----------------------------------------------------------------------------

void ZmodemSession::sendFrameHeader(unsigned char type, long pos)
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

void ZmodemSession::sendBin32FrameHeader(unsigned char type, long pos)
{
	frame32_t frame;
    memset(&frame, 0, sizeof(frame_t));
    frame.type = type;
	frame.flag[ZP0] = pos;
	frame.flag[ZP1] = pos>>8;
	frame.flag[ZP2] = pos>>16;
	frame.flag[ZP3] = pos>>24;
    sendBin32Frame(frame);
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
    LOG_INFO(getSessionName() 
        << "[" << getSessionId() << "] " << getCurState().getName() << " "
        << "sent Bin32 frame:" << getTypeStr(frame.type));
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
					sendFrameHeader(ZACK, zmodemFileM->getPos());
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

void ZmodemSession::sendZrinit()
{
	frame_t frame;
    memset(&frame, 0, sizeof(frame_t));
    frame.type = ZRINIT;
    frame.flag[ZF0] = CANFC32|CANFDX|CANOVIO;
	sendFrame(frame);
}

//-----------------------------------------------------------------------------

void ZmodemSession::handleZfile()
{
    if (inputFrameM->type == ZCOMPL){
        asynHandleEvent(SKIP_EVT);
        return;
    }
	if (inputFrameM->type != ZFILE){
        asynHandleEvent(DESTROY_EVT);
        return;
    }

    if (bufferLenM - decodeIndexM > 1024){
        asynHandleEvent(DESTROY_EVT);
        return;
    }

    int ret = parseZdata();
    if (ret == 0) {return;}
    if (ret < 0) {
        asynHandleEvent(DESTROY_EVT);
        return;
    }

    int nullCount = 0;
    for(unsigned i = decodeIndexM; i < bufferLenM; i++){
        if (bufferM[i] == 0){nullCount++;}
        if (nullCount >= 2){break;}
    }
    if (nullCount < 2){return;}

	std::string filename(curBuffer());
	decodeIndexM += filename.length() + 1;
	std::string fileinfo(curBuffer());
	decodeIndexM += fileinfo.length() + 1;

	decodeIndexM = lastCheckExcapedM;//crc len
	if (*curBuffer() == XON){
		decodeIndexM++;
	}

	eatBuffer();
	recvLenM = 0;

	if (zmodemFileM)
		delete zmodemFileM;

	zmodemFileM = new ZmodemFile("./", filename, fileinfo);
	sendFrameHeader(ZRPOS, zmodemFileM->getPos());
    asynHandleEvent(NEXT_EVT);
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

void ZmodemSession::sendFileInfo()
{
	//USES_CONVERSION;
	//base::PlatformFileInfo info;
	//bool res = GetFileInfo(uploadFilePath_, &info);
	//std::string basename(W2A(uploadFilePath_.BaseName().value().c_str()));
	//if (res == false){
	//	std::string out(std::string("can't get info of file:") + basename + "");
	//	LOG_SE_ERROR(out.c_str());
	//	handleEvent(RESET_EVT);
	//	return;
	//}
	//if (info.size >= 0x100000000) {
	//	LOG_SE_ERROR("The file size[%llu] is larger than %lu(max in 4 bytes defined in zmodem)!", info.size, 0xFFFFFFFF);
	//	handleEvent(RESET_EVT);
	//	return;
	//}
    std::string basename;
	char filedata[1024] = {0};
	unsigned filedata_len = 0;
	memcpy(filedata + filedata_len, basename.c_str(), basename.length() +1);
	filedata_len += basename.length() +1;
	//snprintf(filedata + filedata_len, sizeof(filedata_len) - filedata_len, "%lu %lo 100644 0 1 %lu", 
	//	(long)info.size, (long)(info.last_modified.ToInternalValue()/1000000), (long)info.size);
	filedata_len += strlen(filedata + filedata_len);
	filedata[filedata_len++] = 0;

	frame32_t frame;
	frame.type = ZFILE;
	frame.flag[ZF0] = ZCBIN;	/* file conversion request */
	frame.flag[ZF1] = ZF1_ZMCLOB;	/* file management request */
	frame.flag[ZF2] = 0;	/* file transport request */
	frame.flag[ZF3] = 0;
	sendBin32Frame(frame);
	send_zsda32(filedata, filedata_len, ZCRCW);

	if (zmodemFileM){
		delete zmodemFileM;
		zmodemFileM = NULL;
	}
	//zmodemFileM = new ZmodemFile(W2A(uploadFilePath_.value().c_str()), basename, info.size);
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

void ZmodemSession::handleFlowCntl()
{
	while (decodeIndexM < bufferLenM)
	{
		char code = bufferM[decodeIndexM];
		if (code == XOFF) {
			decodeIndexM++;
			eatBuffer();
			handleEvent(SEND_ZDATA_LATER_EVT);
		}
		else if (code == XON)
		{
			decodeIndexM++;
			eatBuffer();
		}
		else
		{
			asynHandleEvent(CHECK_FRAME_TYPE_EVT);
			return;
		}
	}
}

//-----------------------------------------------------------------------------

void ZmodemSession::handleZdata()
{
	if (inputFrameM->type != ZDATA){
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
					if (!zmodemFileM->write(curBuffer(), lastCheckExcapedSavedM - decodeIndexM)){
						handleEvent(RESET_EVT);
						return;
					}
					lastCheckExcapedSavedM = lastCheckExcapedM;
					decodeIndexM = lastCheckExcapedM+1;
                    eatBuffer();
					dataCrcM = 0xFFFFFFFFL;
                    asynHandleEvent(NETWORK_INPUT_EVT);
                    return;
				}else {
                    LOG_SE_ERROR("ZCRCG crc error! at len:" << (lastCheckExcapedSavedM - decodeIndexM));
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
					if (!zmodemFileM->write(curBuffer(), lastCheckExcapedSavedM - decodeIndexM)){
						handleEvent(RESET_EVT);
						return;
					}
					lastCheckExcapedSavedM = lastCheckExcapedM;
					decodeIndexM = lastCheckExcapedM+1;
					dataCrcM = 0xFFFFFFFFL;
                    eatBuffer();
					sendFrameHeader(ZACK, zmodemFileM->getPos());
					continue;
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
					if (!zmodemFileM->write(curBuffer(), lastCheckExcapedSavedM - decodeIndexM)){
						handleEvent(RESET_EVT);
						return;
					}
					lastCheckExcapedSavedM = lastCheckExcapedM;
					decodeIndexM = lastCheckExcapedM+1;
					dataCrcM = 0xFFFFFFFFL;
                    eatBuffer();
					sendFrameHeader(ZACK, zmodemFileM->getPos());
					continue;
				}
			}else{
				lastCheckExcapedM++;
				bufferM[lastCheckExcapedSavedM] = bufferM[lastCheckExcapedM] ^ 0x40;
				dataCrcM = UPDC32((unsigned char)(bufferM[lastCheckExcapedSavedM]), dataCrcM);
			}
		}else{
			bufferM[lastCheckExcapedSavedM] = bufferM[lastCheckExcapedM];
			dataCrcM = UPDC32((unsigned char)(bufferM[lastCheckExcapedSavedM]), dataCrcM);
		}
	}
	eatBuffer();
    // wait more data
	return;
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendZrpos()
{
	sendFrameHeader(ZRPOS, zmodemFileM->getPos());
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

void ZmodemSession::sendOO(nd::Session* session)
{
    ZmodemSession* self = (ZmodemSession*)session;
	if (self->inputFrameM->type != ZFIN){
        self->asynHandleEvent(DESTROY_EVT);
        return;
    }
    const char* oo = "OO";
    g_stdout->sendData(oo, strlen(oo));
    self->asynHandleEvent(RESET_EVT);
}

//-----------------------------------------------------------------------------


