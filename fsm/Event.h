/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#ifndef EVENT_H
#define EVENT_H

#include "Action.h"

//#define FSM_EVENT(ID, ACTION) nd::Event((#ID), (ID), nd::Action(ACTION))
typedef void (nd::Session::* SessionFunc)();
#define FSM_EVT_S(ID, ACTION, ...) nd::Event((#ID), (ID), FSM_BIND((SessionFunc)ACTION, _1, ##__VA_ARGS__))
#define FSM_EVENT(ID, ACTION, ...) nd::Event((#ID), (ID), FSM_BIND(ACTION, _1, ##__VA_ARGS__))

namespace nd
{
    class Event
    {
    public:

        Event(const std::string& theEvtName, const int theId, Action theAction)
            : nameM(theEvtName)
            , idM(theId)
            , actionM(theAction)
        {}
        ~Event(){};
        
        std::string getName() const
        {
            return nameM;
        }

        int getId() const
        {
            return idM;
        }

        Action getAction() const
        {
            return actionM;
        }
    private:
        std::string nameM;
        int idM;
        Action actionM;

    };
}
#endif /* EVENT_H */

