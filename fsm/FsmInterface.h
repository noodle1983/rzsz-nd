#ifndef FSMINTERFACE_H
#define FSMINTERFACE_H

#define NOMINMAX

#include "Processor.h"

#include <functional>
using namespace std::placeholders;
#define FSM_BIND std::bind
#define FSM_FUNCTION std::function

#define ASYN_PROCESS_EVT(...) g_processor->PROCESS(__VA_ARGS__)

#endif
