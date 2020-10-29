#include "ZmodemSession.h"
#include "Processor.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

using namespace nd;
using namespace std;

int main(int argc, char const *argv[])
{
    auto session = new ZmodemSession();

    session->asynHandleEvent(ZmodemSession::RESET_EVT);
    g_processor->run();

    return 0;
}
