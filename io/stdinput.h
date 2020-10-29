#include "Singleton.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

namespace nd 
{
    class StdInput
    {
    public:
        StdInput(){
            fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
        }
        virtual ~StdInput(){}

        int getInput(const char* buf, unsigned len){
            return read(0, buf, len);
        }
    };

}

#define g_stdin nd::Singleton<nd::StdInput>::instance()

