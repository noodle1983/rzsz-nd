/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#include "SfSession.h"
#include "crctab.h"
#include "zmodem.h"
#include "OscPackage_generated.h"
#include "osc.h"
#include "stdoutput.h"
#include "string.h"
#include "Version.h"
#include "base64.h"
#include <assert.h>

#include <iostream>

//-----------------------------------------------------------------------------

static std::shared_ptr<nd::FiniteStateMachine> g_sz_fsm;
nd::FiniteStateMachine* SfSession::getZmodemFsm()
{
    if (NULL == g_sz_fsm.get())
    {
        nd::FiniteStateMachine* fsm = new nd::FiniteStateMachine;
        (*fsm) += FSM_STATE(INIT_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,      SE_FUNC(OscSession, initState));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,      SE_FUNC(OscSession, sendInitReq));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,      NEW_TIMER(1500));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,    SE_FUNC(OscSession, unsupportNotice));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,    CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(RESET_EVT,      CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, SE_FUNC(OscSession, parsePkg));
        (*fsm) +=      FSM_EVENT(NEXT_EVT,       SE_FUNC(OscSession, sendEmptyDirs));
        (*fsm) +=      FSM_EVENT(NEXT_EVT,       CHANGE_STATE(SEND_FILE_INFO_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,    CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,       CANCEL_TIMER());

        (*fsm) += FSM_STATE(SEND_FILE_INFO_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         SE_FUNC(OscSession, sendFileInfo));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(3000));
        (*fsm) +=      FSM_EVENT(NEXT_EVT,          CHANGE_STATE(SEND_FILE_INFO_STATE));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(OK_EVT,            CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(RESET_EVT,         CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, SE_FUNC(OscSession, parsePkg));
        (*fsm) +=      FSM_EVENT(SEND_DATA_EVT,     CHANGE_STATE(SEND_DATA_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(SEND_DATA_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         SE_FUNC(OscSession, sendFileContent));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(3000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(SEND_DATA_EVT,     CHANGE_STATE(SEND_DATA_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, SE_FUNC(OscSession, parsePkg));
        (*fsm) +=      FSM_EVENT(RESET_EVT,         CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(NEXT_EVT,          CHANGE_STATE(SEND_FILE_INFO_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(END_STATE);	
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         SE_FUNC(OscSession, sendBye));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(1000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(DESTROY_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, SE_FUNC(OscSession, parsePkg));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(DESTROY_STATE));
        (*fsm) +=      FSM_EVENT(RESET_EVT,         CHANGE_STATE(DESTROY_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(DESTROY_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         SE_FUNC(OscSession, destroy));
        (*fsm) +=      FSM_EVENT(RESET_EVT,         IGNORE_EVT());
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       &OscSession::deleteSelf);

        g_sz_fsm.reset(fsm);
    }

    return g_sz_fsm.get();
}

//-----------------------------------------------------------------------------

SfSession::SfSession()
	: OscSession(getZmodemFsm())
{
    handleEvent(ENTRY_EVT);
}

//-----------------------------------------------------------------------------

SfSession::~SfSession(){
    for(auto file : filesM){
        delete file;
    }
    filesM.clear();
}

//-----------------------------------------------------------------------------

void SfSession::sf(std::vector<ZmodemFile*>& files)
{
    filesM.insert(filesM.end(), files.begin(), files.end());
    if (curStateIdM == IDLE_STATE){
        asynHandleEvent(SEND_FILE_EVT);
    }
}

//-----------------------------------------------------------------------------

