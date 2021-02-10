/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
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
