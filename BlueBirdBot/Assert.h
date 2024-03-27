#pragma once
#include <iostream>
#include <string>

#ifndef RELEASE
#   define CORE_ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` \nfailed in " << __FILE__ \
                      << " \nline " << __LINE__ << ": " << message << std::endl; \
            std::terminate(); \
        } \
    } while (false)
#else // ifdef RELEASE
#   define CORE_ASSERT(condition, message)
#endif // ifndef RELEASE