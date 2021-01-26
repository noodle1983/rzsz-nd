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

        void send_data(const char* buffer, const int len)
        {
            int writed = write(STDOUT_FILENO, buffer, len);
            assert(writed == len);
        }
    };

}

#define g_stdout nd::Singleton<nd::StdOutput>::instance()

#endif /* OUTPUT_H */

