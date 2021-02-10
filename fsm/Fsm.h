/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#ifndef FSM_H
#define FSM_H

#include "State.h"
#include "Log.h"
#include <map>

namespace nd
{
    class FiniteStateMachine
    {
    public:
        enum {STATE_NONE = -1};
        FiniteStateMachine()
            : initStateIdM(STATE_NONE)
            , endStateIdM(STATE_NONE)
        {
        }

        ~FiniteStateMachine(){}

        FiniteStateMachine& operator+=(const State& theState);
        FiniteStateMachine& operator+=(const Event& theEvent);
        const std::string& getEventName(const int theEventName);

        inline State& getState(const int theStateId)
        {
            std::map<int, State>::iterator it = statesM.find(theStateId);
            if (it == statesM.end())
            {
                LOG_FATAL("failed to find state with id:" << theStateId);
                exit(-1);
            }
            return it->second;
        }

        inline int getFirstStateId()
        {
            return initStateIdM;
        }

        inline int getLastStateId()
        {
            return endStateIdM;
        }

    private:
        std::map<int, State> statesM;
        std::map<int, std::string> eventNamesM;
        int initStateIdM;
        int endStateIdM;
    };

}
#endif /* FSM_H */

