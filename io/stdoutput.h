#ifndef OUTPUT_H
#define OUTPUT_H

#include "Singleton.hpp"
#include "Log.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

namespace nd 
{
    class StdOutput
    {
    public:
        StdOutput()
        {
        }

        virtual ~StdOutput(){
        }

        void waitUntilWritable(){
            while(true){
                fd_set fds;
                struct timeval timeout;
                timeout.tv_sec = 0;
                timeout.tv_usec = 10*1000;
                FD_SET(STDOUT_FILENO, &fds);
                if(select(STDOUT_FILENO + 1, NULL, &fds, NULL, &timeout)<0){
                    LOG_ERROR("stdout select failed! errno:" << errno);
                    return;
                }
                if (!FD_ISSET(STDOUT_FILENO, &fds)){continue;}

                return;
            }
        }

        void sendData(const char* buffer, const int len)
        {
            int writeIndex = 0;
            do{
                int dataLen = len - writeIndex;
                int writed = write(STDOUT_FILENO, buffer + writeIndex, dataLen);
                if (writed > 0){
                    writeIndex += writed;
                }

                if (writed != dataLen){
                    waitUntilWritable();
                }
            }while(writeIndex < len);
        }
    };

}

#define g_stdout nd::Singleton<nd::StdOutput>::instance()

#endif /* OUTPUT_H */

