#include "RzSession.h"
#include "crctab.h"
#include "zmodem.h"
#include "stdoutput.h"
#include "string.h"
#include <assert.h>

#include <iostream>

//-----------------------------------------------------------------------------

static std::shared_ptr<nd::FiniteStateMachine> g_rz_fsm;
nd::FiniteStateMachine* RzSession::getZmodemFsm()
{
    if (NULL == g_rz_fsm.get())
    {
        nd::FiniteStateMachine* fsm = new nd::FiniteStateMachine;
        (*fsm) += FSM_STATE(IDLE_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,      SE_FUNC(ZmodemSession, initState));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,      SE_FUNC(RzSession, sendLeadingMsg));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,      CHANGE_STATE(SEND_ZRINIT_STATE));

        (*fsm) += FSM_STATE(SEND_ZRINIT_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         SE_FUNC(ZmodemSession, sendClientWorkingDir));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         SE_FUNC(ZmodemSession, sendZrinit));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(60000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(RESET_EVT,         CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, SE_FUNC(ZmodemSession, parseFrame));
        (*fsm) +=      FSM_EVENT(HANDLE_FRAME_EVT,  CHANGE_STATE(HANDLE_ZFILE_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(HANDLE_ZFILE_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         SE_FUNC(ZmodemSession, handleZfile));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(3000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(SKIP_EVT,          CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, CHANGE_STATE(HANDLE_ZFILE_STATE));
        (*fsm) +=      FSM_EVENT(NEXT_EVT,          CHANGE_STATE(WAIT_ZDATA_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(WAIT_ZDATA_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(3000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, SE_FUNC(ZmodemSession, parseFrame));
        (*fsm) +=      FSM_EVENT(HANDLE_FRAME_EVT,  SE_FUNC(ZmodemSession, waitZdata));
        (*fsm) +=      FSM_EVENT(NEXT_EVT,          CHANGE_STATE(HANDLE_ZDATA_STATE));
        (*fsm) +=      FSM_EVENT(SKIP_EVT,          CHANGE_STATE(HANDLE_ZFILE_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(HANDLE_ZDATA_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         SE_FUNC(ZmodemSession, handleZdata));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(3000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(RESET_EVT,         CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, CHANGE_STATE(HANDLE_ZDATA_STATE));
        (*fsm) +=      FSM_EVENT(NEXT_EVT,          CHANGE_STATE(ZEOF_STATE));
        (*fsm) +=      FSM_EVENT(EXIT_EVT,          CANCEL_TIMER());

        (*fsm) += FSM_STATE(ZEOF_STATE);
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         SE_FUNC(ZmodemSession, parseFrame));
        (*fsm) +=      FSM_EVENT(ENTRY_EVT,         NEW_TIMER(3000));
        (*fsm) +=      FSM_EVENT(TIMEOUT_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(DESTROY_EVT,       CHANGE_STATE(END_STATE));
        (*fsm) +=      FSM_EVENT(NETWORK_INPUT_EVT, SE_FUNC(ZmodemSession, parseFrame));
        (*fsm) +=      FSM_EVENT(HANDLE_FRAME_EVT,  CHANGE_STATE(SEND_ZRINIT_STATE));
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

        g_rz_fsm.reset(fsm);
    }

    return g_rz_fsm.get();
}

//-----------------------------------------------------------------------------

RzSession::RzSession()
	: ZmodemSession(getZmodemFsm())
{
    asynHandleEvent(ENTRY_EVT);
}

//-----------------------------------------------------------------------------

RzSession::~RzSession(){
}

//-----------------------------------------------------------------------------

void RzSession::sendLeadingMsg()
{
    const char* msg = "rz waiting to receive.\r";
    g_stdout->sendData(msg, strlen(msg));
}

//-----------------------------------------------------------------------------




