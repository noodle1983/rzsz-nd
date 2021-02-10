/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#include "CppProcessor.h"
#include "CppWorker.h"

#include <functional>
#include <chrono>

using namespace std;
using namespace nd;

//-----------------------------------------------------------------------------

CppProcessor::CppProcessor(const unsigned theThreadCount)
    : threadCountM(theThreadCount)
    , workersM(NULL)
{
}

//-----------------------------------------------------------------------------

CppProcessor::CppProcessor(const std::string& theName, const unsigned theThreadCount)
    : threadCountM(theThreadCount)
    , workersM(NULL)
    , nameM(theName)
    , waitStopM(false)
{
}

//-----------------------------------------------------------------------------

CppProcessor::~CppProcessor()
{
    if (workersM) {
        if (waitStopM){
            waitStop();
        }
        else {
            stop();
        }
    }
}

//-----------------------------------------------------------------------------

void CppProcessor::start(bool toWaitStop)
{
    waitStopM = toWaitStop;
    if (0 == threadCountM)
        return;

    if (NULL != workersM)
        return;

    workersM = new CppWorker[threadCountM];
    threadsM.reserve(threadCountM);
    for (unsigned i = 0; i < threadCountM; i++)
    {
        //workersM[i].setGroupInfo(threadCountM, i);
        threadsM.push_back(thread(&CppWorker::run, &workersM[i]));
    }
}

//-----------------------------------------------------------------------------

void CppProcessor::waitStop()
{
    lock_guard<mutex> lock(stopMutexM);
    if (NULL == workersM)
        return;

    unsigned int i = 0;
    while(true)
    {
        /* check the worker once only */
        if(i < threadCountM && workersM[i].isJobQueueEmpty())
        {
            workersM[i].waitStop();
            i++;
        }
        if (i == threadCountM)
        {
            break;
        }
        else
        {
            this_thread::sleep_for(chrono::milliseconds(1));
        }
    }
    for (unsigned i = 0; i < threadCountM; i++)
    {
        threadsM[i].join();
    }
    delete []workersM;
    workersM = NULL;
}

//-----------------------------------------------------------------------------

void CppProcessor::stop()
{
    lock_guard<mutex> lock(stopMutexM);
    if (NULL == workersM)
        return;

    for (unsigned i = 0; i < threadCountM; i++)
    {
        workersM[i].stop();
    }
    for (unsigned i = 0; i < threadCountM; i++)
    {
        threadsM[i].join();
    }
    delete []workersM;
    workersM = NULL;
}

//-----------------------------------------------------------------------------
