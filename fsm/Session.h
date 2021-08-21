/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#ifndef SESSION_H
#define SESSION_H

#include "Fsm.h"
#include "State.h"
#include "FsmInterface.h"
#include "min_heap.h"

#include <stdint.h>
#include <string>

namespace nd
{
    class Session
    {
    public:
        enum PreDefineEvent
        {
            ENTRY_EVT     = -1,
            EXIT_EVT      = -2,
            TIMEOUT_EVT   = -3,
            NEXT_EVT      = -4,
            OK_EVT        = -5, 
            FAILED_EVT    = -6

        };

        Session(
                FiniteStateMachine* theFsm, 
                const uint64_t theSessionId);
        Session();
        void init(
                FiniteStateMachine* theFsm, 
                const uint64_t theSessionId);
        virtual ~Session();


        State& toNextState(const int theNextStateId);
		int asynHandleEvent(const int theEventId);
        void handleEvent(const int theEventId);
        void newTimer(const long theMsec);
        void handleTimeout();
        void cancelTimer();
        void resetTimer();


        const uint64_t getSessionId()
        {
            return sessionIdM;
        }

        virtual const char* getSessionName()
        {
            return "Session";
        }

        const std::string& getEventName(const int theEventName)
        {
            return fsmM->getEventName(theEventName);
        }

        inline State& getEndState()
        {
            return fsmM->getState(endStateIdM);
        }

        inline State& getCurState()
        {
            return fsmM->getState(curStateIdM);
        }

        bool isToDelete(){return isDestroyedM;}
        void setDelete(){isDestroyedM = true;}

    protected:
        bool isInitializedM;
        bool isDestroyedM;
        unsigned char curStateIdM;
        unsigned char endStateIdM;
        unsigned timerIdM;
        FiniteStateMachine* fsmM;

        long prevTimerIntervalM;
        min_heap_item_t* fsmTimerM;
        uint64_t sessionIdM;

    };
}

#endif /* SESSION_H */
