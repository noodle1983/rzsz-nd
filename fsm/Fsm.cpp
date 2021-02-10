/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#include "Fsm.h"

using namespace nd;
//-----------------------------------------------------------------------------

FiniteStateMachine& FiniteStateMachine::operator+=(const State& theState)
{
    int stateId = theState.getId();
    if (STATE_NONE == initStateIdM)
    {
        initStateIdM = stateId;
    }
    if (statesM.find(stateId) != statesM.end())
    {
        LOG_FATAL("exit because fsm state " << theState.getName()
                << " is redefined!");
        exit(-1);
    }
    endStateIdM = stateId;

    statesM[theState.getId()] = theState;
    return *this;
}

//-----------------------------------------------------------------------------

FiniteStateMachine& FiniteStateMachine::operator+=(const Event& theEvent)
{
    eventNamesM[theEvent.getId()] = theEvent.getName();
    State& curState = statesM[endStateIdM];
    curState.addEvent(theEvent);
    return *this;
}

//-----------------------------------------------------------------------------

const std::string& FiniteStateMachine::getEventName(const int theEventName)
{
    std::map<int, std::string>::iterator it = eventNamesM.find(theEventName);
    if (it != eventNamesM.end())
    {
        return it->second;
    }
    else
    {
        static const std::string emptyString; 
        return emptyString;
    }
}

//-----------------------------------------------------------------------------

