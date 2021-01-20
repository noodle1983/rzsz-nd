#include "Action.h"
#include "Session.h"
#include "State.h"

#include <string>

using namespace Fsm;
//-----------------------------------------------------------------------------

void Fsm::changeState(
        Fsm::Session* theSession,
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

void Fsm::generateEvent(
        Fsm::Session* theSession,
        const int theEventId)
{
    theSession->handleEvent(theEventId);
}

//-----------------------------------------------------------------------------

void Fsm::ignoreEvent(
        Fsm::Session* theSession)
{
    State& curState = theSession->getCurState();
    LOG_DEBUG(theSession->getSessionName() 
            << "[" << theSession->getSessionId() << "] " << curState.getName()
            << " ignore event.");
}

//-----------------------------------------------------------------------------
void Fsm::newTimer(
        Fsm::Session* theSession,
        const long theMsec)
{
    theSession->newTimer(theMsec);
}

//-----------------------------------------------------------------------------

void Fsm::newFuncTimer(
		Fsm::Session* theSession,
		TimerGettor theTimerGettor)
{
	long msec = theTimerGettor();
	return Fsm::newTimer(theSession, msec);
}

//-----------------------------------------------------------------------------
void Fsm::cancelTimer(
        Fsm::Session* theSession)
{
    theSession->cancelTimer();
}

//-----------------------------------------------------------------------------

void Fsm::deleteSession(
        Fsm::Session* theSession)
{
    delete theSession;
}

//-----------------------------------------------------------------------------

