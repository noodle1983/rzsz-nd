#include "SzSession.h"
#include "crctab.h"
#include "zmodem.h"
#include "stdoutput.h"
#include "string.h"
#include "Version.h"
#include <assert.h>

#include <iostream>

//-----------------------------------------------------------------------------

static std::shared_ptr<nd::FiniteStateMachine> g_sz_fsm;
nd::FiniteStateMachine* SzSession::getZmodemFsm()
{
    if (NULL == g_sz_fsm.get())
    {
        nd::FiniteStateMachine* fsm = new nd::FiniteStateMachine;
        (*fsm) += FSM_STATE(IDLE_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,      SE_FUNC(ZmodemSession, initState));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,      NEW_TIMER(1000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,    CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(SEND_FILE_EVT,  CHANGE_STATE(SEND_ZRQINIT_STATE));
        (*fsm) +=      FSM_EVENT(RESET_EVT,      CHANGE_STATE(IDLE_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,    CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,       CANCEL_TIMER());
        (*fsm) +=      FSM_EVENT(EXIT_EVT,       SE_FUNC(SzSession, sendLeadingMsg));

        (*fsm) += FSM_STATE(SEND_ZRQINIT_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         SE_FUNC(SzSession, sendZrqinit));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(3000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(RESET_EVT,         CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, SE_FUNC(ZmodemSession, parseFrame));
        (*fsm) +=      FSM_EVENT(HANDLE_FRAME_EVT,  CHANGE_STATE(SEND_ZFILE_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(SEND_ZFILE_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         SE_FUNC(SzSession, sendZfile));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(3000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(SEND_ZDATA_EVT,    CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(SKIP_EVT,          CHANGE_STATE(SEND_ZRQINIT_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, SE_FUNC(ZmodemSession, parseFrame));
        (*fsm) +=      FSM_EVENT(HANDLE_FRAME_EVT,  CHANGE_STATE(HANDLE_ZFILE_RSP_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(HANDLE_ZFILE_RSP_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         SE_FUNC(ZmodemSession, handleZfileRsp));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(3000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(SKIP_EVT,          CHANGE_STATE(SEND_ZRQINIT_STATE));
        (*fsm) +=      FSM_EVENT(SEND_ZDATA_EVT,    CHANGE_STATE(SEND_ZDATA_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(SEND_ZDATA_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         SE_FUNC(ZmodemSession, sendZdata));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(3000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(SEND_ZDATA_EVT,    CHANGE_STATE(SEND_ZDATA_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, SE_FUNC(ZmodemSession, parseFrame));
        (*fsm) +=      FSM_EVENT(HANDLE_FRAME_EVT,  CHANGE_STATE(SEND_ZFILE_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(END_STATE);	
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         SE_FUNC(ZmodemSession, sendZfin));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(1000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(DESTROY_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, SE_FUNC(ZmodemSession, parseFrame));
        (*fsm) +=      FSM_EVENT(HANDLE_FRAME_EVT,  SE_FUNC(ZmodemSession, sendOO));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(DESTROY_STATE));
        (*fsm) +=      FSM_EVENT(RESET_EVT,         CHANGE_STATE(DESTROY_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(DESTROY_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         SE_FUNC(ZmodemSession, destroy));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       &ZmodemSession::deleteSelf);

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
    for(auto file : filesM){
        delete file;
    }
    filesM.clear();
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

void SzSession::sendLeadingMsg()
{
    const char* msg = "rz\r";
    g_stdout->sendData(msg, strlen(msg));
}

//-----------------------------------------------------------------------------

void SzSession::sendZrqinit()
{
    if(filesM.empty()){
        asynHandleEvent(DESTROY_EVT);
        return;
    }
    
	frame32_t frame;
	frame.type = ZRQINIT;
	frame.flag[ZF0] = 0;
	frame.flag[ZF1] = 0;
	frame.flag[ZF2] = 0;
	frame.flag[ZF3] = ZVERSION;
	sendBin32Frame(frame);
    startInputTimer();
}

//-----------------------------------------------------------------------------

void SzSession::sendZfile()
{
	if (inputFrameM.type != ZRINIT || filesM.size() == 0){
        asynHandleEvent(DESTROY_EVT);
        return;
    }
    peerVersionM = inputFrameM.flag[ZF3];
    
	if (zmodemFileM){
		delete zmodemFileM;
		zmodemFileM = NULL;
	}
    zmodemFileM = filesM.back();
    filesM.pop_back();

	if (zmodemFileM->getFileSize() >= 0x100000000 && peerVersionM == 0) {
		LOG_SE_ERROR("The file[" << zmodemFileM->getFilename()
                << "] size[" << zmodemFileM->getFileSize() 
                << "] is larger than the maximum in 4 bytes defined in zmodem)!");
		reportM << std::endl;
        reportM << "The file[" << zmodemFileM->getFilename()
                << "] size[" << zmodemFileM->getFileSize() 
                << "] is larger than the maximum in 4 bytes defined in zmodem)!";
		reportM << std::endl;
        reportM << "To make it possible, you may need the newest version of putty-nd(https://noodle1983.github.io/tags/putty-nd).";
		reportM << std::endl;
        asynHandleEvent(DESTROY_EVT);
		return;
	}

    const std::string& basename = zmodemFileM->getFilename();
	char filedata[1024] = {0};
	unsigned filedata_len = 0;
	memcpy(filedata + filedata_len, basename.c_str(), basename.length());
	filedata_len += basename.length();
	filedata[filedata_len++] = 0;
	snprintf(filedata + filedata_len, sizeof(filedata) - filedata_len, "%llu %lo 100644 0 1 %llu", 
		zmodemFileM->getFileSize(),
        (long)zmodemFileM->getFileTime(),
        zmodemFileM->getFileSize());
	filedata_len += strlen(filedata + filedata_len);
	filedata[filedata_len++] = 0;

	frame32_t frame;
	frame.type = ZFILE;
	frame.flag[ZF0] = ZCBIN;	/* file conversion request */
	frame.flag[ZF1] = ZF1_ZMCLOB;	/* file management request */
	frame.flag[ZF2] = 0;	/* file transport request */
	frame.flag[ZF3] = ZVERSION;
	sendBin32Frame(frame);
    //Pathname\0Length ModificationDate FileMode SerialNumber NumberOfFilesRemaining NumberOfBytesRemaining FileType
	send_zsda32(filedata, filedata_len, ZCRCW);

    LOG_SE_INFO("send ZFILE. filename:" << basename << ", file info:" << filedata[basename.length() + 1])
}

//-----------------------------------------------------------------------------

