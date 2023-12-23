/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
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
#define SE_FUNC(theType, theFunc) FSM_BIND(nd::wrapSessionFunction<theType>, _1, &theType::theFunc)
#define SHOW_GROGRESSS(theMsg)    FSM_BIND(nd::showProgress,  _1, (theMsg))
#define CLEAR_GROGRESSS(theMsg)   FSM_BIND(nd::clearProgress,  _1, (theMsg))

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

    void showProgress(
            nd::Session* theSession,
            const char* theMsg);

    void clearProgress(
            nd::Session* theSession,
            const char* theMsg);


    template<typename SessionType>
    void wrapSessionFunction(
            nd::Session* theSession,
            void (SessionType::*func)()){
        SessionType* self = (SessionType*)theSession;
        (self->*func)();
    }
}

#endif /* ACTION_H */
