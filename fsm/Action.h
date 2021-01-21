#ifndef ACTION_H
#define ACTION_H

#include "FsmInterface.h"

#define CHANGE_STATE(theStateId)  FSM_BIND(nd::changeState,   _1, (theStateId))
#define GEN_EVT(theEventId)       FSM_BIND(nd::generateEvent, _1, (theEventId))
#define NEW_TIMER(theMsec)        FSM_BIND(nd::newTimer,      _1, (theMsec))
#define NEW_FUNC_TIMER(theGettor) FSM_BIND(nd::newFuncTimer,  _1, nd::TimerGettor(theGettor))
#define CANCEL_TIMER()            (&nd::cancelTimer)
#define DELETE_SESSION()          (&nd::deleteSession)
#define IGNORE_EVT()              (&nd::ignoreEvent)

namespace nd
{
    class Session;
    typedef FSM_FUNCTION<void (nd::Session *)> Action;
    typedef FSM_FUNCTION<long ()> TimerGettor;

    void changeState(
            nd::Session* theSession,
            const int theNextStateId);

    void generateEvent(
            nd::Session* theSession,
            const int theEventId);

    void ignoreEvent(
            nd::Session* theSession);

    void newTimer(
            nd::Session* theSession,
            const long theMsec);

    void newFuncTimer(
            nd::Session* theSession,
            TimerGettor theTimerGettor);

    void cancelTimer(
            nd::Session* theSession);

    void deleteSession(
            nd::Session* theSession);
}

#endif /* ACTION_H */
