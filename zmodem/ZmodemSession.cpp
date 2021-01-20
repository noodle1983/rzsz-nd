#include "ZmodemSession.h"
#include "crctab.h"
#include "zmodem.h"
#include "stdinput.h"
#include "stdoutput.h"
#include "string.h"
#include <assert.h>

//-----------------------------------------------------------------------------

ZmodemSession::ZmodemSession(Fsm::FiniteStateMachine* fsm)
	: Fsm::Session(fsm, 0)
	, lastEscaped_(false)
	, isDestroyed_(false)
    , zmodemFile_(NULL)
    , inputTimerM(NULL)
{
	inputFrame_ = new frame_t;
	sendFinOnReset_ = false;
	isSz_ = true;
	file_select_state_ = FILE_SELECT_NONE;
	tick_ = 0;

	int i;
	for (i=0;i<256;i++) {	
		if (i & 0140){
			zsendline_tab[i]=0;
		}else {
			switch(i)
			{
			case ZDLE:
			case XOFF: /* ^Q */
			case XON: /* ^S */
			case (XOFF | 0200):
			case (XON | 0200):
				zsendline_tab[i]=1;
				break;
			case 020: /* ^P */
			case 0220:
				zsendline_tab[i]=1;
				break;
			case 015:
			case 0215:
				zsendline_tab[i]=0;
				break;
			default:
				zsendline_tab[i]=0;
			}
		}
	}
}

//-----------------------------------------------------------------------------

ZmodemSession::~ZmodemSession()
{
	delete inputFrame_;
    stopInputTimer();
}

//-----------------------------------------------------------------------------

void ZmodemSession::initState()
{
	if (/*zmodemFile_ ||*/ sendFinOnReset_){
		delete zmodemFile_;
		zmodemFile_ = NULL;
		if ((!isSz_) && inputFrame_->type == ZFIN){
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
	buffer_.clear();
	buffer_.reserve(1024 * 16);
	decodeIndex_ = 0;
	lastCheckExcaped_ = 0;
	lastCheckExcapedSaved_ = 0;
	dataCrc_ = 0xFFFFFFFFL;
	recv_len_ = 0;
	lastEscaped_ = false;
	sendFinOnReset_ = false;
	//uploadFilePath_.clear();
	file_select_state_ = FILE_SELECT_NONE;
	tick_ = 0;
	return;
}

//-----------------------------------------------------------------------------

void ZmodemSession::checkFrametype()
{
	for (; decodeIndex_ < buffer_.length() 
		&& ZPAD != buffer_[decodeIndex_] ; decodeIndex_ ++);
	for (; decodeIndex_ < buffer_.length() 
		&& ZPAD == buffer_[decodeIndex_] ; decodeIndex_ ++);

	if (decodeIndex_ + 2 >= buffer_.length()){
		return;
	}

	if (ZDLE != buffer_[decodeIndex_++]){
		handleEvent(RESET_EVT);
		return;
	}

	int frametype = buffer_[decodeIndex_++];
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
	if (decodeIndex_ + sizeof(hex_t) >= buffer_.length())
		return;
	hex_t  hexframe;
	memcpy(&hexframe, curBuffer(), sizeof(hex_t));
	decodeIndex_ += sizeof(hex_t);

	frame_t frame;
    convHex2Plain(&hexframe, &frame);
    if (frame.crc != calcFrameCrc(&frame)){
		output("\r\ncrc error!\r\n");
        handleEvent(RESET_EVT);
        return ;
    }

	unsigned old_index = decodeIndex_;
	for (; decodeIndex_ < buffer_.length() 
		&& ('\r' == buffer_[decodeIndex_] 
			|| '\n' == buffer_[decodeIndex_]
			|| -118 == buffer_[decodeIndex_]) ; decodeIndex_ ++);
	if (old_index == decodeIndex_){
		output("\r\nno line seed found!\r\n");
        handleEvent(RESET_EVT);
        return ;
    }
	if (frame.type != ZACK && frame.type != ZFIN && buffer_[decodeIndex_++] != XON){
		output("\r\nXON expected!\r\n");
        handleEvent(RESET_EVT);
        return ;
    }
	eatBuffer();
	memcpy(inputFrame_, &frame, sizeof(frame_t));
    LOG_INFO(getSessionName() 
        << "[" << getSessionId() << "] " << getCurState().getName() << " "
        << "got Hex frame:" << getTypeStr(inputFrame_->type));
	handleEvent(HANDLE_FRAME_EVT);
	return;
}

//-----------------------------------------------------------------------------

void ZmodemSession::parseBinFrame()
{
	if (decodeIndex_ + sizeof(frame_t) >= buffer_.length())
		return;
	frame_t frame;
	int frame_len = 0;
    if (!decodeEscapeStruct<frame_t>(decodeIndex_, frame_len, frame)){
		return;
	}
	decodeIndex_ += frame_len;

    if (frame.crc != calcFrameCrc(&frame)){
		output("\r\nbin crc error!\r\n");
        handleEvent(RESET_EVT);
        return ;
    }
	eatBuffer();
	memcpy(inputFrame_, &frame, sizeof(frame_t));
    LOG_INFO(getSessionName() 
        << "[" << getSessionId() << "] " << getCurState().getName() << " "
        << "got Bin frame:" << getTypeStr(inputFrame_->type));
	handleEvent(HANDLE_FRAME_EVT);
	return;
}

//-----------------------------------------------------------------------------

void ZmodemSession::parseBin32Frame()
{
	if (decodeIndex_ + sizeof(frame32_t) > buffer_.length())
		return;
	frame32_t frame;
	int frame_len = 0;
	if (!decodeEscapeStruct<frame32_t>(decodeIndex_, frame_len, frame)){
		return;
	}
	decodeIndex_ += frame_len;

    if (frame.crc != calcFrameCrc32(&frame)){
		output("\r\nbin32 crc error!\r\n");
        handleEvent(RESET_EVT);
        return ;
    }
	eatBuffer();
	memcpy(inputFrame_, &frame, sizeof(frame_t));
    LOG_INFO(getSessionName() 
        << "[" << getSessionId() << "] " << getCurState().getName() << " "
        << "got Bin32 frame:" << getTypeStr(inputFrame_->type));
	handleEvent(HANDLE_FRAME_EVT);
	return;
}

//-----------------------------------------------------------------------------

void ZmodemSession::handleFrame()
{
	switch (inputFrame_->type){
    case ZRQINIT:
        return sendZrinit();

    case ZFILE: 
		isSz_ = true;
		return handleZfile();
    case ZDATA:
		return handleZdata();
    case ZEOF:
		if (zmodemFile_){
			delete zmodemFile_;
			zmodemFile_ = NULL;
		}
		return sendZrinit();
    case ZFIN:
		sendFinOnReset_ = true;
		handleEvent(RESET_EVT);
		return;
    case ZRINIT:
		isSz_ = false;
		if (file_select_state_ == FILE_SELECT_NONE){
			file_select_state_ = FILE_SELECTING;
			//PuttyFileDialogSingleton::instance()->showOpenDialog(
			//	frontend_->getNativeParentWindow(), this);
			sendFinOnReset_ = true;
			//no timer for user to select file
			cancelTimer();
		}else if (file_select_state_ == FILE_SELECTED && zmodemFile_ && !zmodemFile_->isGood()){
			//complete or send other files;
			sendBin32FrameHeader(ZCOMPL, 0);
			file_select_state_ = FILE_SELECT_NONE;;
			handleEvent(RESET_EVT);
		}
		return;
	case ZRPOS: {
			if (!zmodemFile_) {
				sendFinOnReset_ = true;
				sendBin32FrameHeader(ZCAN, 0);
				sendBin32FrameHeader(ZABORT, 0);
				handleEvent(RESET_EVT);
				return;
			}
			unsigned pos = getPos(inputFrame_);
			output("\r\nremote set pos to %d\r\n", pos);
			zmodemFile_->setPos(pos);
			sendBin32FrameHeader(ZDATA, zmodemFile_->getPos());
			sendZdata();
		}
		return;
    case ZNAK:
		if (decodeIndex_ < buffer_.length()){
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

	//if (frontend_->send_buffer_size() > 1024*1024){
	//	if (!isToDelete()) asynHandleEvent(SEND_ZDATA_LATER_EVT);
	//	return;
	//}

	unsigned len = zmodemFile_->read(buffer, BUFFER_LEN);
	char frameend = zmodemFile_->isGood() ? ZCRCG : ZCRCE;
	send_zsda32(buffer, len, frameend);
	std::string report_line(zmodemFile_->getProgressLine());
	//flow_control_fresh_lastline(frontend_->term, zmodemFile_->getPrompt().length(), 
    //		report_line.c_str(), report_line.length());
		
	if(!zmodemFile_->isGood()){
		sendBin32FrameHeader(ZEOF, zmodemFile_->getPos());
		//term_fresh_lastline(frontend_->term, zmodemFile_->getPrompt().length(), 
		//		report_line.c_str(), report_line.length());
		//output("\r\nflushing remaining buffer(%d) to server...", frontend_->send_buffer_size());
		return;
	}else{
		if (!isToDelete()) asynHandleEvent(SEND_ZDATA_EVT);
		return;
	}
}

//-----------------------------------------------------------------------------

void ZmodemSession::onSentTimeout()
{
	if (!zmodemFile_->isGood()) {
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
	unsigned oldIndex = decodeIndex_;
	std::string filename(curBuffer());
	decodeIndex_ += filename.length() + 1;
	std::string fileinfo(curBuffer());
	decodeIndex_ += fileinfo.length() + 1;

	if (decodeIndex_ + 6 > buffer_.length()){
		decodeIndex_ = oldIndex;
        handleEvent(WAIT_DATA_EVT);
        return ;
	}
	int crc_len = 0;
	unsigned long recv_crc = decodeCrc32(decodeIndex_ + 2, crc_len);
	if (crc_len == 0){
		decodeIndex_ = oldIndex;
		handleEvent(WAIT_DATA_EVT);
        return ;
	}
	buffer_[decodeIndex_] = buffer_[decodeIndex_+1];
	decodeIndex_++;
	unsigned long crc = calcBufferCrc32(buffer_.c_str() + oldIndex, decodeIndex_ - oldIndex);

	decodeIndex_++;
	decodeIndex_ += crc_len;
	if (*curBuffer() == XON){
		decodeIndex_++;
	}

	if (recv_crc != crc){
		output("\r\nzfile frame crc invalid!\r\n");
		sendFinOnReset_ = true;
        handleEvent(RESET_EVT);
        return ;
	}
	eatBuffer();
	recv_len_ = 0;

	if (zmodemFile_)
		delete zmodemFile_;

	//zmodemFile_ = new ZmodemFile(default_output_path, filename, fileinfo);
	//std::string report_line(zmodemFile_->getProgressLine());
	//term_fresh_lastline(frontend_->term, 0, 
	//	report_line.c_str(), report_line.length());

	sendFrameHeader(ZRPOS, zmodemFile_->getPos());
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

	if (zmodemFile_){
		delete zmodemFile_;
		zmodemFile_ = NULL;
	}
	//zmodemFile_ = new ZmodemFile(W2A(uploadFilePath_.value().c_str()), basename, info.size);
	//std::string report_line(zmodemFile_->getProgressLine());
	//term_fresh_lastline(frontend_->term, 0, 
	//	report_line.c_str(), report_line.length());
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
	while (decodeIndex_ < buffer_.length())
	{
		char code = buffer_[decodeIndex_];
		if (code == XOFF) {
			decodeIndex_++;
			eatBuffer();
			handleEvent(SEND_ZDATA_LATER_EVT);
		}
		else if (code == XON)
		{
			decodeIndex_++;
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
	//curBuffer() with len buffer_.length() - decodeIndex_
	//offset in inputFrame_

	for (; lastCheckExcaped_ < buffer_.length() - 1; lastCheckExcaped_++, lastCheckExcapedSaved_++){
		if (buffer_[lastCheckExcaped_] == ZDLE){
			if (lastCheckExcaped_ + 6 > buffer_.length()){
				handleEvent(WAIT_DATA_EVT);
				return;
			}
			if (ZCRCE == buffer_[lastCheckExcaped_ + 1]){
				unsigned long calc_crc = ~UPDC32(buffer_[lastCheckExcaped_+1], dataCrc_);
				int consume_len = 0;
				unsigned long recv_crc = decodeCrc32(lastCheckExcaped_+2, consume_len);
				if (consume_len == 0){
					handleEvent(WAIT_DATA_EVT);
					return;
				}

				if (calc_crc == recv_crc){
					lastCheckExcaped_ += 1 + consume_len;
					if (!zmodemFile_->write(curBuffer(), lastCheckExcapedSaved_ - decodeIndex_)){
						handleEvent(RESET_EVT);
						return;
					}
					lastCheckExcapedSaved_ = lastCheckExcaped_;
					decodeIndex_ = lastCheckExcaped_+1;
					dataCrc_ = 0xFFFFFFFFL;
					
					std::string report_line(zmodemFile_->getProgressLine());
					//term_fresh_lastline(frontend_->term, zmodemFile_->getPrompt().length(), 
					//	report_line.c_str(), report_line.length());
					//term_data(frontend_->term, 0, "\r\n", 2);
					handleEvent(NETWORK_INPUT_EVT);
					return;
				}
			}else if (ZCRCG == buffer_[lastCheckExcaped_ + 1]){	
				unsigned long calc_crc = ~UPDC32((unsigned char)(buffer_[lastCheckExcaped_+1]), dataCrc_);
				int consume_len = 0;
				unsigned long recv_crc = decodeCrc32(lastCheckExcaped_+2, consume_len);
				if (consume_len == 0){
					handleEvent(WAIT_DATA_EVT);
					return;
				}

				if (calc_crc == recv_crc){
					assert(lastCheckExcapedSaved_- decodeIndex_ == 1024);
					lastCheckExcaped_ += 1 + consume_len;
					if (!zmodemFile_->write(curBuffer(), lastCheckExcapedSaved_ - decodeIndex_)){
						handleEvent(RESET_EVT);
						return;
					}
					lastCheckExcapedSaved_ = lastCheckExcaped_;
					decodeIndex_ = lastCheckExcaped_+1;
					dataCrc_ = 0xFFFFFFFFL;
					continue;
				}else {
					handleEvent(RESET_EVT);
					return;
				}
				//else it is normal char

			}else if (ZCRCQ == buffer_[lastCheckExcaped_ + 1]){
				unsigned long calc_crc = ~UPDC32(buffer_[lastCheckExcaped_+1], dataCrc_);
				int consume_len = 0;
				unsigned long recv_crc = decodeCrc32(lastCheckExcaped_+2, consume_len);
				if (consume_len == 0){
					handleEvent(WAIT_DATA_EVT);
					return;
				}

				if (calc_crc == recv_crc){
					lastCheckExcaped_ += 1 + consume_len;
					if (!zmodemFile_->write(curBuffer(), lastCheckExcapedSaved_ - decodeIndex_)){
						handleEvent(RESET_EVT);
						return;
					}
					lastCheckExcapedSaved_ = lastCheckExcaped_;
					decodeIndex_ = lastCheckExcaped_+1;
					dataCrc_ = 0xFFFFFFFFL;
					sendFrameHeader(ZNAK, zmodemFile_->getPos());
					continue;
				}

			}else if (ZCRCW == buffer_[lastCheckExcaped_ + 1]){
				unsigned long calc_crc = ~UPDC32(buffer_[lastCheckExcaped_+1], dataCrc_);
				int consume_len = 0;
				unsigned long recv_crc = decodeCrc32(lastCheckExcaped_+2, consume_len);
				if (consume_len == 0){
					handleEvent(WAIT_DATA_EVT);
					return;
				}

				if (calc_crc == recv_crc){
					lastCheckExcaped_ += 1 + consume_len;
					if (!zmodemFile_->write(curBuffer(), lastCheckExcapedSaved_ - decodeIndex_)){
						handleEvent(RESET_EVT);
						return;
					}
					lastCheckExcapedSaved_ = lastCheckExcaped_;
					decodeIndex_ = lastCheckExcaped_+1;
					dataCrc_ = 0xFFFFFFFFL;
					sendFrameHeader(ZNAK, zmodemFile_->getPos());
					continue;
				}
			}else{
				lastCheckExcaped_++;
				buffer_[lastCheckExcapedSaved_] = buffer_[lastCheckExcaped_] ^ 0x40;
				dataCrc_ = UPDC32((unsigned char)(buffer_[lastCheckExcapedSaved_]), dataCrc_);
			}
		}else{
			buffer_[lastCheckExcapedSaved_] = buffer_[lastCheckExcaped_] ;
			dataCrc_ = UPDC32((unsigned char)(buffer_[lastCheckExcapedSaved_]), dataCrc_);
		}
	}
	eatBuffer();
	//zmodemFile_->write(curBuffer(), len);
	//decodeIndex_ += len;
	//eatBuffer();
	///recv_len_ += len;
	std::string report_line(zmodemFile_->getProgressLine());
	//flow_control_fresh_lastline(frontend_->term, zmodemFile_->getPrompt().length(),
	//	report_line.c_str(), report_line.length());
	handleEvent(WAIT_DATA_EVT);
	return;
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendZrpos()
{
	sendFrameHeader(ZRPOS, zmodemFile_->getPos());
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
	buffer_.append(str, len);
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

void ZmodemSession::deleteSelf(Fsm::Session* session)
{
    delete session;
    g_processor->waitStop();
}

//-----------------------------------------------------------------------------

void ZmodemSession::parseFrame(Fsm::Session* session)
{
    ZmodemSession* self = (ZmodemSession*)session;
    self->checkFrametype();
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendZFIN(Fsm::Session* session)
{
    ZmodemSession* self = (ZmodemSession*)session;
    frame_t frame;
    memset(&frame, 0, sizeof(frame_t));
    frame.type = ZFIN;
    self->sendFrame(frame);
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendOO(Fsm::Session* session)
{
    ZmodemSession* self = (ZmodemSession*)session;
	if (self->inputFrame_->type != ZFIN){
        self->asynHandleEvent(DESTROY_EVT);
        return;
    }
    const char* oo = "OO";
    send_data(oo, strlen(oo));
    self->asynHandleEvent(RESET_EVT);
}

//-----------------------------------------------------------------------------

void ZmodemSession::sendZDATA(Fsm::Session* session)
{
    ZmodemSession* self = (ZmodemSession*)session;
    self->sendZdata();
}

//void ZmodemSession::flow_control_fresh_lastline(Terminal *term, int headerlen, const char *data, int len)
//{
//	uint64_t now = GetTickCount64();
//	uint64_t diff = now - tick_;
//	bool ignore = ((now / 100) % 10) >= 8; //20% must flow control
//	if ((ignore || (now - tick_ > 120)) && bufchain_size(&term->inbuf) == 0){
//		tick_ = now;
//		term_fresh_lastline(term, headerlen, data, len);
//	}
//	else
//	{
//		tick_ = tick_;
//	}
//}

//-----------------------------------------------------------------------------
