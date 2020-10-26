#ifndef CPPWORKER_H
#define CPPWORKER_H

#include <functional>
#include <condition_variable>
#include <list>
#include <thread>

#include <min_heap.h>

typedef void (*TimerCallback)(void *arg);

namespace nd
{
    typedef std::function<void ()> Job;
    typedef std::list<Job*> JobQueue;
    class CppWorker
    {
    public:
        CppWorker();
        ~CppWorker();

        inline bool isJobQueueEmpty()
        {
            std::lock_guard<std::mutex> lock(queueMutexM);
            return jobQueueM.empty();
        }
		inline unsigned getQueueSize()
		{
            std::lock_guard<std::mutex> lock(queueMutexM);
            return jobQueueM.size();
        }
        void stop();
        void waitStop();

        void process(Job* theJob);
		min_heap_item_t* addLocalTimer(
				uint64_t theMsTime, 
				TimerCallback theCallback,
				void* theArg);
		void cancelLocalTimer(min_heap_item_t*& theEvent);

        void run();

		void handleLocalTimer();

    private:
        JobQueue jobQueueM;
        std::mutex queueMutexM;
        std::mutex nullMutexM;
        std::condition_variable queueCondM;

		//integrate timer handling
		min_heap_t timerHeapM;

        volatile size_t isToStopM;
        volatile size_t isWaitStopM;
    };
}

#endif /* CPPWORKER_H */

