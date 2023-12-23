/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#include "Action.h"
#include "Session.h"
#include "State.h"
#include "ProgressWin.h"

#include <string>

using namespace nd;
//-----------------------------------------------------------------------------

void nd::changeState(
        nd::Session* theSession,
        const int theNextStateId)
{
    //exec EXIT FUNC
    {
        State& curState = theSession->getCurState();

        ActionList& actionList = curState.getActionList(Session::EXIT_EVT);
        ActionList::iterator it = actionList.begin();
        if (it != actionList.end())
        {
            LOG_DEBUG(theSession->getSessionName() 
                    << "[" << theSession->getSessionId() << "] " << curState.getName() << " handleEvent("
                    << theSession->getEventName(Session::EXIT_EVT) << ")");
            for (; it != actionList.end(); it++)
            {
                Action action = *it;
				action(theSession);
            }
        }
    }

    //exec ENTRY_EVT
    {
        State& nextState = theSession->toNextState(theNextStateId);

        ActionList& actionList = nextState.getActionList(Session::ENTRY_EVT);
        ActionList::iterator it = actionList.begin();
        if (it != actionList.end())
        {
            LOG_DEBUG(theSession->getSessionName() 
                    << "[" << theSession->getSessionId() << "] " << nextState.getName() << " handleEvent("
                    << theSession->getEventName(Session::ENTRY_EVT) << ")");
            for (; it != actionList.end(); it++)
            {
                Action action = *it;
				action(theSession);
            }
        }
    }
    return ;
}

//-----------------------------------------------------------------------------

void nd::generateEvent(
        nd::Session* theSession,
        const int theEventId)
{
    theSession->handleEvent(theEventId);
}

//-----------------------------------------------------------------------------

void nd::ignoreEvent(
        nd::Session* theSession)
{
    State& curState = theSession->getCurState();
    LOG_DEBUG(theSession->getSessionName() 
            << "[" << theSession->getSessionId() << "] " << curState.getName()
            << " ignore event.");
}

//-----------------------------------------------------------------------------
void nd::newTimer(
        nd::Session* theSession,
        const long theMsec)
{
    theSession->newTimer(theMsec);
}

//-----------------------------------------------------------------------------

void nd::newFuncTimer(
		nd::Session* theSession,
		TimerGettor theTimerGettor)
{
	long msec = theTimerGettor();
	return nd::newTimer(theSession, msec);
}

//-----------------------------------------------------------------------------
void nd::cancelTimer(
        nd::Session* theSession)
{
    theSession->cancelTimer();
}

//-----------------------------------------------------------------------------

void nd::deleteSession(
        nd::Session* theSession)
{
    delete theSession;
}

//-----------------------------------------------------------------------------

void nd::showProgress(
        nd::Session* theSession,
        const char* theMsg)
{
    g_progress_win->addMsg(theMsg);
}

//-----------------------------------------------------------------------------

void nd::clearProgress(
        nd::Session* theSession,
        const char* theMsg)
{
    g_progress_win->clearMsg(theMsg);
}

//-----------------------------------------------------------------------------
