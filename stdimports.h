//
// Created by bknun on 1/6/2017.
//

#ifndef SHARP_STDIMPORTS_H
#define SHARP_STDIMPORTS_H

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <list>
#include <stdexcept>
#include <string>
#include <cstdio>
#include <cstring>
#include <thread>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <math.h>

//#define CONTROLLER
#ifdef CONTROLLER
#include "controller/generated/function_mapping.h"
#else
#include "flight_controller/generated/function_mapping.h"
#endif

using namespace std;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define WIN32_
#else
#define POSIX_
#endif


#define POSIX_USEC_INTERVAL 1000
#define CXX11_INLINE inline
#define TIME_SINCE(time) (micros() - time)


#endif //SHARP_STDIMPORTS_H
