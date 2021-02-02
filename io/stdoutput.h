#ifndef OUTPUT_H
#define OUTPUT_H

#include "Singleton.hpp"
#include "Log.h"
#include "zmodem.h"
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

        void sendData(const char* buffer, const int len)
        {
            int writeIndex = 0;
            do{
                int dataLen = len - writeIndex;
                int written = write(STDOUT_FILENO, buffer + writeIndex, dataLen);
                if (written > 0){
                    writeIndex += written;
                }

                if (written != dataLen){
                    waitUntilWritable(STDOUT_FILENO);
                }
            }while(writeIndex < len);
        }
    };

}

#define g_stdout nd::Singleton<nd::StdOutput>::instance()

#endif /* OUTPUT_H */

