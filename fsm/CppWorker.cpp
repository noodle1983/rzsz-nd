#include "CppWorker.h"
#include "Log.h"
#include "min_heap.h"

#include <assert.h>

using namespace nd;
using namespace std;

//-----------------------------------------------------------------------------
CppWorker::CppWorker()
    : isToStopM(false)
    , isWaitStopM(false)
{
    min_heap_ctor(&timerHeapM);	
}

//-----------------------------------------------------------------------------

CppWorker::~CppWorker()
{
    min_heap_dtor(&timerHeapM);	
}

//-----------------------------------------------------------------------------

void CppWorker::stop()
{
    isToStopM = true;
    queueCondM.notify_one();
}

//-----------------------------------------------------------------------------

void CppWorker::waitStop()
{
    isWaitStopM = true;
    queueCondM.notify_one();
}

//-----------------------------------------------------------------------------


void CppWorker::process(Job* theJob)
{
    bool jobQueueEmpty = false;
    {
        lock_guard<mutex> lock(queueMutexM);
        jobQueueEmpty = jobQueueM.empty();
        jobQueueM.push_back(theJob);
    }
    if (jobQueueEmpty)
    {
        queueCondM.notify_one();
    }
}

//-----------------------------------------------------------------------------

min_heap_item_t* CppWorker::addLocalTimer(
        uint64_t theMsTime, 
		TimerCallback theCallback,
		void* theArg)
{
	bool timerHeapEmpty = min_heap_empty(&timerHeapM);
    if (128 > min_heap_size(&timerHeapM))
    {
        min_heap_reserve(&timerHeapM, 128);
    }

	min_heap_item_t* timeoutEvt = new min_heap_item_t();
    timeoutEvt->callback = theCallback;
    timeoutEvt->arg = theArg;
    timeoutEvt->timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(theMsTime);

    if (-1 == min_heap_push(&timerHeapM, timeoutEvt))
    {
        LOG_FATAL("not enough memory!");
        exit(-1);
    }
	if (timerHeapEmpty)
	{
        queueCondM.notify_one();
	}
	return timeoutEvt;
}

//-----------------------------------------------------------------------------

void CppWorker::cancelLocalTimer(min_heap_item_t*& theEvent)
{
    if (theEvent == NULL) {return;}
    min_heap_erase(&timerHeapM, theEvent);
    delete theEvent;
	theEvent = NULL;
}

//-----------------------------------------------------------------------------

void CppWorker::handleLocalTimer()
{
    if (!min_heap_empty(&timerHeapM))
    {
        auto timeNow = std::chrono::steady_clock::now();
        while(!min_heap_empty(&timerHeapM)) 
        {
            min_heap_item_t* topEvent = min_heap_top(&timerHeapM);
            if (item_cmp(topEvent->timeout, timeNow, <=))
            {
                min_heap_pop(&timerHeapM);
                (topEvent->callback)(topEvent->arg);
                delete topEvent;
            }
            else
            {
                break;
            }
        } 
    }


}

//-----------------------------------------------------------------------------

void CppWorker::run()
{
    Job* job;
    while (!isToStopM)
    {
        job = NULL;
        {
            lock_guard<mutex> lock(queueMutexM);
            if (!jobQueueM.empty())
            {
                job = jobQueueM.front();
                jobQueueM.pop_front();
            }
            else if(isWaitStopM)
            {
                break;
            }
        }

        //handle Job
        if (job != NULL)
        {
            (*job)();
            delete job;
        }

        //handle timer
        handleLocalTimer();

        unique_lock<mutex> queueLock(queueMutexM);
		if (!jobQueueM.empty()) { continue; }

        if (!isToStopM && !isWaitStopM && jobQueueM.empty() && !min_heap_empty(&timerHeapM))
        {
            queueCondM.wait_for(queueLock, chrono::microseconds(500));
        }
        else
        {
            while (!isToStopM && !isWaitStopM && jobQueueM.empty() && min_heap_empty(&timerHeapM))
            {
                queueCondM.wait(queueLock); 
            }
        }
    }
}

