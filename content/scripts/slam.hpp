#pragma once
#ifndef SLAM_HPP
#define SLAM_HPP

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #define S_EXPORT EMSCRIPTEN_KEEPALIVE
#else
    #define S_EXPORT
#endif

#include <string>
#include <cmath>
#include <map>

namespace Slam 
{
    // Export helpers
    #define SFUNCTION(ReturnType, Name, ...) \
        extern "C" S_EXPORT ReturnType Name(__VA_ARGS__)

    #define SPROPERTY(Type, Name) \
        extern "C" S_EXPORT Type Name

    SFUNCTION(void, StartLoop) {}
}

#endif // SLAM_HPP
