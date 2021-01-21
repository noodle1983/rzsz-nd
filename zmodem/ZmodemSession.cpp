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
	, isDestroyedM(false)
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
			send_data("OO", 2);
		}else{
			frame_t frame;
			memset(&frame, 0, sizeof(frame_t));
			frame.type = ZFIN;
			sendFrame(frame);
		}

		output("\r\n");
		if (!isToDelete()) asynHandleEvent(RESET_EVT);
	}
	bufferM.clear();
	bufferM.reserve(1024 * 16);
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

void ZmodemSession::checkFrametype()
{
	for (; decodeIndexM < bufferM.length() 
		&& ZPAD != bufferM[decodeIndexM] ; decodeIndexM ++);
	for (; decodeIndexM < bufferM.length() 
		&& ZPAD == bufferM[decodeIndexM] ; decodeIndexM ++);

	if (decodeIndexM + 2 >= bufferM.length()){
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
		//output("\r\nonly support(HEX,BIN,BIN32) frame\r\n");
		handleEvent(RESET_EVT);
		return;
	}
}

//-----------------------------------------------------------------------------

void ZmodemSession::parseHexFrame()
{
	if (decodeIndexM + sizeof(hex_t) >= bufferM.length())
		return;
	hex_t  hexframe;
	memcpy(&hexframe, curBuffer(), sizeof(hex_t));
	decodeIndexM += sizeof(hex_t);

	frame_t frame;
    convHex2Plain(&hexframe, &frame);
    if (frame.crc != calcFrameCrc(&frame)){
		output("\r\ncrc error!\r\n");
        handleEvent(RESET_EVT);
        return ;
    }

	unsigned old_index = decodeIndexM;
	for (; decodeIndexM < bufferM.length() 
		&& ('\r' == bufferM[decodeIndexM] 
			|| '\n' == bufferM[decodeIndexM]
			|| -118 == bufferM[decodeIndexM]) ; decodeIndexM ++);
	if (old_index == decodeIndexM){
		output("\r\nno line seed found!\r\n");
        handleEvent(RESET_EVT);
        return ;
    }
	if (frame.type != ZACK && frame.type != ZFIN && bufferM[decodeIndexM++] != XON){
		output("\r\nXON expected!\r\n");
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
	if (decodeIndexM + sizeof(frame_t) >= bufferM.length())
		return;
	frame_t frame;
	int frame_len = 0;
    if (!decodeEscapeStruct<frame_t>(decodeIndexM, frame_len, frame)){
		return;
	}
	decodeIndexM += frame_len;

    if (frame.crc != calcFrameCrc(&frame)){
		output("\r\nbin crc error!\r\n");
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
	if (decodeIndexM + sizeof(frame32_t) > bufferM.length())
		return;
	frame32_t frame;
	int frame_len = 0;
	if (!decodeEscapeStruct<frame32_t>(decodeIndexM, frame_len, frame)){
		return;
	}
	decodeIndexM += frame_len;

    if (frame.crc != calcFrameCrc32(&frame)){
		output("\r\nbin32 crc error!\r\n");
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
			output("\r\nremote set pos to %d\r\n", pos);
			zmodemFileM->setPos(pos);
			sendBin32FrameHeader(ZDATA, zmodemFileM->getPos());
			sendZdata();
		}
		return;
    case ZNAK:
		if (decodeIndexM < bufferM.length()){
			handleEvent(NETWORK_INPUT_EVT);
		}
		return;
    case ZSINIT:
		output("\r\nunexpected frame type:ZSINIT\r\n");
		break;
    case ZACK:
		output("\r\nunexpected frame type:ZACK\r\n");
		break;
    case ZSKIP:
		output("\r\nunexpected frame type:ZSKIP. no permission to write file?\r\n");
		break;
    case ZABORT:
		output("\r\nunexpected frame type:ZABORT\r\n");
		break;
    case ZFERR:
		output("\r\nunexpected frame type:ZFERR\r\n");
		break;
    case ZCRC:
		output("\r\nunexpected frame type:ZCRC\r\n");
		break;
    case ZCHALLENGE:
		output("\r\nunexpected frame type:ZCHALLENGE\r\n");
		break;
    case ZCOMPL:
		output("\r\nunexpected frame type:ZCOMPL\r\n");
		break;
    case ZCAN:
		output("\r\nunexpected frame type:ZCAN\r\n");
		break;
    case ZFREECNT:
		output("\r\nunexpected frame type:ZFREECNT\r\n");
		break;
    case ZCOMMAND:
		output("\r\nunexpected frame type:ZCOMMAND\r\n");
		break;
    case ZSTDERR:
		output("\r\nunexpected frame type:ZSTDERR\r\n");
		break;
    default:
        output("\r\ninvalid frame type!\r\n");
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
		//output(".");
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
    send_data(buf, len);
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
    send_data(buf, len);
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
	unsigned oldIndex = decodeIndexM;
	std::string filename(curBuffer());
	decodeIndexM += filename.length() + 1;
	std::string fileinfo(curBuffer());
	decodeIndexM += fileinfo.length() + 1;

	if (decodeIndexM + 6 > bufferM.length()){
		decodeIndexM = oldIndex;
        handleEvent(WAIT_DATA_EVT);
        return ;
	}
	int crc_len = 0;
	unsigned long recv_crc = decodeCrc32(decodeIndexM + 2, crc_len);
	if (crc_len == 0){
		decodeIndexM = oldIndex;
		handleEvent(WAIT_DATA_EVT);
        return ;
	}
	bufferM[decodeIndexM] = bufferM[decodeIndexM+1];
	decodeIndexM++;
	unsigned long crc = calcBufferCrc32(bufferM.c_str() + oldIndex, decodeIndexM - oldIndex);

	decodeIndexM++;
	decodeIndexM += crc_len;
	if (*curBuffer() == XON){
		decodeIndexM++;
	}

	if (recv_crc != crc){
		output("\r\nzfile frame crc invalid!\r\n");
		sendFinOnResetM = true;
        handleEvent(RESET_EVT);
        return ;
	}
	eatBuffer();
	recvLenM = 0;

	if (zmodemFileM)
		delete zmodemFileM;

	//zmodemFileM = new ZmodemFile(default_output_path, filename, fileinfo);

	sendFrameHeader(ZRPOS, zmodemFileM->getPos());
}

//-----------------------------------------------------------------------------

void ZmodemSession::send_zsda32(char *buf, size_t length, char frameend)
{
	char send_buf[2048+128];
	size_t send_len = 0;
	unsigned long crc;

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
	send_data(send_buf, send_len);
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendFileInfo()
{
	//USES_CONVERSION;
	//base::PlatformFileInfo info;
	//bool res = GetFileInfo(uploadFilePath_, &info);
	//std::string basename(W2A(uploadFilePath_.BaseName().value().c_str()));
	//if (res == false){
	//	std::string out(std::string("can't get info of file:") + basename + "\r\n");
	//	output(out.c_str());
	//	handleEvent(RESET_EVT);
	//	return;
	//}
	//if (info.size >= 0x100000000) {
	//	output("\r\nThe file size[%llu] is larger than %lu(max in 4 bytes defined in zmodem)!\r\n", info.size, 0xFFFFFFFF);
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

unsigned short ZmodemSession::decodeCrc(const int index, int& consume_len)
{
	unsigned short ret = 0;
	decodeEscapeStruct<unsigned short>(index, consume_len, ret);
	return ret;
}

//-----------------------------------------------------------------------------

unsigned long ZmodemSession::decodeCrc32(const int index, int& consume_len)
{
	unsigned long ret = 0;
	decodeEscapeStruct<unsigned long>(index, consume_len, ret);
	return ret;
}
//-----------------------------------------------------------------------------

void ZmodemSession::handleFlowCntl()
{
	while (decodeIndexM < bufferM.length())
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
	//curBuffer() with len bufferM.length() - decodeIndexM
	//offset in inputFrameM

	for (; lastCheckExcapedM < bufferM.length() - 1; lastCheckExcapedM++, lastCheckExcapedSavedM++){
		if (bufferM[lastCheckExcapedM] == ZDLE){
			if (lastCheckExcapedM + 6 > bufferM.length()){
				handleEvent(WAIT_DATA_EVT);
				return;
			}
			if (ZCRCE == bufferM[lastCheckExcapedM + 1]){
				unsigned long calc_crc = ~UPDC32(bufferM[lastCheckExcapedM+1], dataCrcM);
				int consume_len = 0;
				unsigned long recv_crc = decodeCrc32(lastCheckExcapedM+2, consume_len);
				if (consume_len == 0){
					handleEvent(WAIT_DATA_EVT);
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
					
					handleEvent(NETWORK_INPUT_EVT);
					return;
				}
			}else if (ZCRCG == bufferM[lastCheckExcapedM + 1]){	
				unsigned long calc_crc = ~UPDC32((unsigned char)(bufferM[lastCheckExcapedM+1]), dataCrcM);
				int consume_len = 0;
				unsigned long recv_crc = decodeCrc32(lastCheckExcapedM+2, consume_len);
				if (consume_len == 0){
					handleEvent(WAIT_DATA_EVT);
					return;
				}

				if (calc_crc == recv_crc){
					assert(lastCheckExcapedSavedM- decodeIndexM == 1024);
					lastCheckExcapedM += 1 + consume_len;
					if (!zmodemFileM->write(curBuffer(), lastCheckExcapedSavedM - decodeIndexM)){
						handleEvent(RESET_EVT);
						return;
					}
					lastCheckExcapedSavedM = lastCheckExcapedM;
					decodeIndexM = lastCheckExcapedM+1;
					dataCrcM = 0xFFFFFFFFL;
					continue;
				}else {
					handleEvent(RESET_EVT);
					return;
				}
				//else it is normal char

			}else if (ZCRCQ == bufferM[lastCheckExcapedM + 1]){
				unsigned long calc_crc = ~UPDC32(bufferM[lastCheckExcapedM+1], dataCrcM);
				int consume_len = 0;
				unsigned long recv_crc = decodeCrc32(lastCheckExcapedM+2, consume_len);
				if (consume_len == 0){
					handleEvent(WAIT_DATA_EVT);
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
					sendFrameHeader(ZNAK, zmodemFileM->getPos());
					continue;
				}

			}else if (ZCRCW == bufferM[lastCheckExcapedM + 1]){
				unsigned long calc_crc = ~UPDC32(bufferM[lastCheckExcapedM+1], dataCrcM);
				int consume_len = 0;
				unsigned long recv_crc = decodeCrc32(lastCheckExcapedM+2, consume_len);
				if (consume_len == 0){
					handleEvent(WAIT_DATA_EVT);
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
					sendFrameHeader(ZNAK, zmodemFileM->getPos());
					continue;
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
	eatBuffer();
	//zmodemFileM->write(curBuffer(), len);
	//decodeIndexM += len;
	//eatBuffer();
	///recvLenM += len;
	handleEvent(WAIT_DATA_EVT);
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
    
    int len = 0;
    while((len = g_stdin->getInput(buffer, sizeof(buffer))) > 0)
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
	bufferM.append(str, len);
	handleEvent(NETWORK_INPUT_EVT);
	return 0;
}

//-----------------------------------------------------------------------------

void ZmodemSession::reset()
{
	const char canistr[] =
	{
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0
	};
	send_data(canistr, strlen(canistr));
	handleEvent(RESET_EVT);
}

//-----------------------------------------------------------------------------

void ZmodemSession::output(const char* str, ...)
{
}

//-----------------------------------------------------------------------------

void ZmodemSession::destroy()
{
	setDelete();
	asynHandleEvent(DESTROY_EVT);
}

//-----------------------------------------------------------------------------

void ZmodemSession::deleteSelf(nd::Session* session)
{
    resetTty(0);
    delete session;
    g_processor->waitStop();
}

//-----------------------------------------------------------------------------

void ZmodemSession::parseFrame(nd::Session* session)
{
    ZmodemSession* self = (ZmodemSession*)session;
    self->checkFrametype();
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendZFIN(nd::Session* session)
{
    ZmodemSession* self = (ZmodemSession*)session;
    frame_t frame;
    memset(&frame, 0, sizeof(frame_t));
    frame.type = ZFIN;
    self->sendFrame(frame);
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
    send_data(oo, strlen(oo));
    self->asynHandleEvent(RESET_EVT);
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendZDATA(nd::Session* session)
{
    ZmodemSession* self = (ZmodemSession*)session;
    self->sendZdata();
}

//-----------------------------------------------------------------------------
