#include <iostream>
#include <functional>

#include "Processor.h"
#include "Log.h"

using namespace std;
using namespace nd;

void on_timeout(void *theArg)
{
    std::cout << "time end" << endl;
    g_processor->waitStop();
}

void say(CppWorker* theProcessor)
{
    std::cout << "Hello, I will exit after 1 second" << std::endl;
    theProcessor->addLocalTimer(1000, on_timeout, NULL);
    std::cout << "time begin" << std::endl;
}

int main(int argc, char *argv[])
{
    g_processor->process(new Job(bind(say, g_processor)));
    g_processor->run();
    return 0;
}

