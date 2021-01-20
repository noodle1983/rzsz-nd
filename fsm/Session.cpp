#include "Session.h"

#include <utility>
#include <Log.h>

using namespace Fsm;

typedef std::pair<Session*, unsigned char> TimerPair;
//-----------------------------------------------------------------------------

Session::Session(
        FiniteStateMachine* theFsm, 
        const uint64_t theSessionId)
    : isInitializedM(false)
    , curStateIdM(theFsm->getFirstStateId())
    , endStateIdM(theFsm->getLastStateId())
    , timerIdM(0)
    , fsmM(theFsm)
    , fsmTimerM(NULL)
    , sessionIdM(theSessionId)
{

}

//-----------------------------------------------------------------------------

Session::Session()
    : isInitializedM(false)
    , curStateIdM(0)
    , endStateIdM(0)
    , timerIdM(0)
    , fsmM(NULL)
    , fsmTimerM(NULL)
    , sessionIdM(0)
{

}

//-----------------------------------------------------------------------------

void Session::init(
        FiniteStateMachine* theFsm, 
        const uint64_t theSessionId)
{
    fsmM = theFsm;
    sessionIdM = theSessionId;
    isInitializedM = false;
    curStateIdM = fsmM->getFirstStateId();
    endStateIdM = fsmM->getLastStateId();
    timerIdM = 0;
    g_processor->cancelLocalTimer(fsmTimerM);
}

//-----------------------------------------------------------------------------

Session::~Session()
{
    g_processor->cancelLocalTimer(fsmTimerM);
}

//-----------------------------------------------------------------------------

int Session::asynHandleEvent(const int theEventId)
{
    ASYN_PROCESS_EVT(&Session::handleEvent, this, theEventId);
    return 0;
}

//-----------------------------------------------------------------------------

void Session::handleEvent(const int theEventId)
{
    if (!isInitializedM)
    {
        //the first state's entry function
        const int curStateId = curStateIdM;
        State& curState = getCurState();
        ActionList& actionList = curState.getActionList(ENTRY_EVT);
        ActionList::iterator it = actionList.begin();
        if (it != actionList.end())
        {
            LOG_DEBUG(getSessionName() 
                    << "[" << getSessionId() << "] " << curState.getName() << " handleEvent("
                    << getEventName(ENTRY_EVT) << ")");
            for (; it != actionList.end(); it++)
            {
                if (curStateId != curStateIdM)
                {
                    LOG_DEBUG("state changed, ignore rest action for event:" << theEventId);
                    break;
                }
				Action action = *it;
				action(this);
            }
        }
        isInitializedM = true;
    }

    State& curState = getCurState();
    LOG_DEBUG(getSessionName() 
            << "[" << getSessionId() << "] " << curState.getName() << " handleEvent("
            << getEventName(theEventId) << ")");

    ActionList& actionList = curState.getActionList(theEventId);
    if (actionList.empty())
    {
        LOG_ERROR(getSessionName()
                << "[" << getSessionId() << "] " << curState.getName()
                << " the Event " << theEventId << " is not defined"
                << " under state:" << curState.getName());
        changeState(this, endStateIdM);
        return ;
    }

    const int curStateId = curStateIdM;
    ActionList::iterator it = actionList.begin();
    for (; it != actionList.end(); it++)
    {
        if (curStateId != curStateIdM)
        {
            LOG_DEBUG(getSessionName()
                << "[" << getSessionId() << "] " << curState.getName()
                << " state changed, ignore rest action for event:" << theEventId);
            break;
        }
        Action action = *it;
		action(this);
    }
    return ;
}

//-----------------------------------------------------------------------------

State& Session::toNextState(const int theNextStateId)
{
    const std::string& preStateName = fsmM->getState(curStateIdM).getName();

    curStateIdM = theNextStateId;
    State& nextState = fsmM->getState(curStateIdM);

    LOG_DEBUG( getSessionName() << "[" << sessionIdM << "] " 
            << preStateName << " -> " << nextState.getName());

    return nextState;
}

//-----------------------------------------------------------------------------

void Session::handleTimeout()
{
    fsmTimerM = NULL; // free by worker
    handleEvent(TIMEOUT_EVT);
}

//-----------------------------------------------------------------------------

void onSessionTimerout(void *arg)
{
    Session* session = (Session*)arg;
    session->handleTimeout();
}

void Session::newTimer(const long theMsec)
{
    g_processor->cancelLocalTimer(fsmTimerM);
	fsmTimerM = g_processor->addLocalTimer(theMsec, onSessionTimerout, this);
}

//-----------------------------------------------------------------------------

void Session::cancelTimer()
{
    g_processor->cancelLocalTimer(fsmTimerM);
}

//-----------------------------------------------------------------------------

