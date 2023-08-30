/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#ifndef OUTPUT_H
#define OUTPUT_H

#include "Singleton.hpp"
#include "ProgressWin.h"
#include "Log.h"
#include "zmodem.h"
#include <iostream>
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
            g_progress_win->updateSentBytes(len);
        }
    };

}

#define g_stdout nd::Singleton<nd::StdOutput>::instance()

#endif /* OUTPUT_H */

