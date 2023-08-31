/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#include "Singleton.hpp"
#include "CppProcessor.h"
#include "KfifoBuffer.h"
#include "ProgressWin.h"
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
            //: processorM(1)
            //, bufferM(20) // 1MB
            //, stopM(false)
        {
            fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
            //processorM.start();
            //processorM.process(1, NEW_JOB(std::bind(&StdInput::read, this)));
        }


        virtual ~StdInput(){
            //stopM = true;
            //processorM.stop();
        }

        int getInput(char* buf, unsigned len){
            //fd_set fds;
            //struct timeval timeout;
            //timeout.tv_sec = 0;
            //timeout.tv_usec = 0;//10*1000;
            //FD_SET(STDIN_FILENO, &fds);
            //if(select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout)<0){
            //    LOG_ERROR("select failed! errno:" << errno);
            //    return 0;
            //}
            //if (!FD_ISSET(STDIN_FILENO, &fds)){return 0;}

            int readed = ::read(STDIN_FILENO, buf, len);
            if (readed > 0){
                g_progress_win->updateRecvBytes(readed);
            }
            return readed;
        }

        //void read() {
        //    fd_set fds;
        //    struct timeval timeout;
        //    timeout.tv_sec = 0;
        //    timeout.tv_usec = 0;//10*1000;
        //    while(!stopM && bufferM.isHealthy()){
        //        FD_SET(0, &fds);
        //        if(select(1, &fds, NULL, NULL, &timeout)<0){
        //            LOG_ERROR("select failed! errno:" << errno);
        //            return;
        //        }
        //        if (!FD_ISSET(0, &fds)){continue;}

        //        char buf[1024];
        //        int len = ::read(0, buf, sizeof(buf));
        //        if (len > 0){
        //            bufferM.putn(buf, len);
        //        }
        //    }
        //}

        //int getInput(char* buf, unsigned len){
        //    bool stopped = !bufferM.isHealthy();
        //    
        //    int readed = bufferM.get(buf, len);
        //    if (stopped && bufferM.isHealthy()){
        //        processorM.process(1, NEW_JOB(std::bind(&StdInput::read, this)));
        //    }
        //    return readed;
        //}

    private:
        //CppProcessor processorM;
        //nd::KfifoBuffer bufferM;
        //std::atomic<bool> stopM;
    };

}

#define g_stdin nd::Singleton<nd::StdInput>::instance()

