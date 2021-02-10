/**
 * Licensing Information
 *    This is a release of rzsz-nd, brought to you by Dong Lu(noodle1983@126
 *    .com). Except for extra permissions from Dong Lu(noodle1983@126.com),
 *    this software is released under version 3 of the GNU General
 *    Public License (GPLv3).
 **/
#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "Singleton.hpp"
#include "CppWorker.h"

#define NEW_JOB(...) (new nd::Job(std::bind(__VA_ARGS__)))
#define PROCESS(...) process(NEW_JOB(__VA_ARGS__))

#define g_processor nd::Singleton<nd::CppWorker, 0>::instance()


#endif /* PROCESSOR_H */

