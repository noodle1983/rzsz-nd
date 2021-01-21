#include "SzSession.h"
#include "crctab.h"
#include "zmodem.h"
#include "stdoutput.h"
#include "string.h"
#include <assert.h>

#include <iostream>

//-----------------------------------------------------------------------------

static std::shared_ptr<Fsm::FiniteStateMachine> g_sz_fsm;
Fsm::FiniteStateMachine* SzSession::getZmodemFsm()
{
    if (NULL == g_sz_fsm.get())
    {
        Fsm::FiniteStateMachine* fsm = new Fsm::FiniteStateMachine;
        (*fsm) += FSM_STATE(IDLE_STATE);
        (*fsm) +=      FSM_EVT_S(ENTRY_EVT,      &ZmodemSession::initState);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,      NEW_TIMER(1000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,    CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(SEND_FILE_EVT,  CHANGE_STATE(SEND_ZRQINIT_STATE));
        (*fsm) +=      FSM_EVENT(RESET_EVT,      CHANGE_STATE(IDLE_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,    CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,       CANCEL_TIMER());
        (*fsm) +=      FSM_EVENT(EXIT_EVT,       &SzSession::sendLeadingMsg);

        (*fsm) += FSM_STATE(SEND_ZRQINIT_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         &SzSession::sendZrqinit);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(3000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(RESET_EVT,         CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, &ZmodemSession::parseFrame);
        (*fsm) +=      FSM_EVENT(HANDLE_FRAME_EVT,  CHANGE_STATE(SEND_ZFILE_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(SEND_ZFILE_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         &SzSession::sendZfile);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(3000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(SKIP_EVT,          CHANGE_STATE(SEND_ZRQINIT_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, &ZmodemSession::parseFrame);
        (*fsm) +=      FSM_EVENT(HANDLE_FRAME_EVT,  CHANGE_STATE(HANDLE_ZFILE_RSP_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(HANDLE_ZFILE_RSP_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         &SzSession::handleZfileRsp);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(3000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(SKIP_EVT,          CHANGE_STATE(SEND_ZRQINIT_STATE));
        (*fsm) +=      FSM_EVENT(SEND_ZDATA_EVT,    CHANGE_STATE(SEND_ZDATA_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(SEND_ZDATA_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         &ZmodemSession::sendZDATA);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(3000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(SEND_ZDATA_EVT,    CHANGE_STATE(SEND_ZDATA_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, &ZmodemSession::parseFrame);
        (*fsm) +=      FSM_EVENT(HANDLE_FRAME_EVT,  CHANGE_STATE(SEND_ZFILE_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(END_STATE);	
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         &ZmodemSession::sendZFIN);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(1000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(DESTROY_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, &ZmodemSession::parseFrame);
        (*fsm) +=      FSM_EVENT(HANDLE_FRAME_EVT,  &ZmodemSession::sendOO);
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(DESTROY_STATE));
        (*fsm) +=      FSM_EVENT(RESET_EVT,         CHANGE_STATE(DESTROY_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(DESTROY_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         &ZmodemSession::deleteSelf);

        g_sz_fsm.reset(fsm);
    }

    return g_sz_fsm.get();
}

//-----------------------------------------------------------------------------

SzSession::SzSession()
	: ZmodemSession(getZmodemFsm())
{
}

//-----------------------------------------------------------------------------

SzSession::~SzSession(){
}

//-----------------------------------------------------------------------------

void SzSession::sz(std::vector<ZmodemFile*>& files)
{
    filesM.insert(filesM.end(), files.begin(), files.end());
    if (curStateIdM == IDLE_STATE){
        asynHandleEvent(SEND_FILE_EVT);
    }
}

//-----------------------------------------------------------------------------

void SzSession::sendLeadingMsg(Fsm::Session* session)
{
    const char* msg = "rz\r";
    send_data(msg, strlen(msg));
}

//-----------------------------------------------------------------------------

void SzSession::sendZrqinit(Fsm::Session* session)
{
    SzSession* self = (SzSession*)session;
    if(self->filesM.empty()){
        self->asynHandleEvent(DESTROY_EVT);
        return;
    }
    
	frame32_t frame;
	frame.type = ZRQINIT;
	frame.flag[ZF0] = 0;
	frame.flag[ZF1] = 0;
	frame.flag[ZF2] = 0;
	frame.flag[ZF3] = 0; // version
	self->sendBin32Frame(frame);
    self->startInputTimer();
}

//-----------------------------------------------------------------------------

void SzSession::sendZfile(Fsm::Session* session)
{
    SzSession* self = (SzSession*)session;
	if (self->inputFrameM->type != ZRINIT || self->filesM.size() == 0){
        self->asynHandleEvent(DESTROY_EVT);
        return;
    }
    
	if (self->zmodemFileM){
		delete self->zmodemFileM;
		self->zmodemFileM = NULL;
	}
    self->zmodemFileM = self->filesM.back();
    self->filesM.pop_back();

    const std::string& basename = self->zmodemFileM->getFilename();
	char filedata[1024] = {0};
	unsigned filedata_len = 0;
	memcpy(filedata + filedata_len, basename.c_str(), basename.length());
	filedata_len += basename.length();
	filedata[filedata_len++] = 0;
	snprintf(filedata + filedata_len, sizeof(filedata_len) - filedata_len, "%llu %lo 100644 0 1 %llu", 
		self->zmodemFileM->getFileSize(),
        (long)self->zmodemFileM->getFileTime(),
        self->zmodemFileM->getFileSize());
	filedata_len += strlen(filedata + filedata_len);
	filedata[filedata_len++] = 0;

	frame32_t frame;
	frame.type = ZFILE;
	frame.flag[ZF0] = ZCBIN;	/* file conversion request */
	frame.flag[ZF1] = ZF1_ZMCLOB;	/* file management request */
	frame.flag[ZF2] = 0;	/* file transport request */
	frame.flag[ZF3] = 0;
	self->sendBin32Frame(frame);
    //Pathname\0Length ModificationDate FileMode SerialNumber NumberOfFilesRemaining NumberOfBytesRemaining FileType
	self->send_zsda32(filedata, filedata_len, ZCRCW);

}

//-----------------------------------------------------------------------------

void SzSession::handleZfileRsp(Fsm::Session* session)
{
    SzSession* self = (SzSession*)session;
    if (self->inputFrameM->type == ZSKIP){
        self->asynHandleEvent(SKIP_EVT);
        return;
    }
	if (self->inputFrameM->type != ZRPOS || self->zmodemFileM == NULL){
        self->asynHandleEvent(DESTROY_EVT);
        return;
    }

    uint64_t pos = getPos(self->inputFrameM);
    LOG_INFO(self->getSessionName() 
        << "[" << self->getSessionId() << "] " << self->getCurState().getName() << " "
        << "got ZRPOS:" << pos);

    self->zmodemFileM->setPos(pos);
    self->sendBin32FrameHeader(ZDATA, pos);
    self->asynHandleEvent(SEND_ZDATA_EVT);
}

//-----------------------------------------------------------------------------


