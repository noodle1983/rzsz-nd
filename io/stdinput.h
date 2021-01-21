#include "Singleton.hpp"
#include "CppProcessor.h"
#include "KfifoBuffer.h"
#include "Log.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <atomic>

namespace nd 
{
    class StdInput
    {
    public:
        StdInput()
            : processorM(1)
            , bufferM(20) // 1MB
            , stopM(false)
        {
            initZmodemTab();
            setvbuf(stdin, NULL, _IONBF, 0);
            fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
            processorM.start();
            processorM.process(1, NEW_JOB(std::bind(&StdInput::read, this)));
        }

        static void alarmHandler(int dummy){}

        virtual ~StdInput(){
            stopM = true;
            signal(SIGALRM, alarmHandler);
            alarm(1);
            processorM.stop();
        }

        void read() {
            fd_set fds;
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 10*1000;
            while(!stopM && bufferM.isHealthy()){
                FD_SET(0, &fds);
                if(select(1, &fds, NULL, NULL, &timeout)<0){
                    LOG_ERROR("select failed! errno:" << errno);
                    return;
                }
                if (!FD_ISSET(0, &fds)){continue;}

                char buf[1024];
                int len = ::read(0, buf, sizeof(buf));
                if (len > 0){
                    bufferM.putn(buf, len);
                }
            }
        }

        int getInput(char* buf, unsigned len){
            bool stopped = !bufferM.isHealthy();
            
            int readed = bufferM.get(buf, len);
            if (stopped && bufferM.isHealthy()){
                processorM.process(1, NEW_JOB(std::bind(&StdInput::read, this)));
            }
            return readed;
        }

    private:
        CppProcessor processorM;
        nd::KfifoBuffer bufferM;
        std::atomic<bool> stopM;
    };

}

#define g_stdin nd::Singleton<nd::StdInput>::instance()

