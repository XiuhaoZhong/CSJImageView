#ifndef __CSJRUNTIME_EXPORT_H__
#define __CSJRUNTIME_EXPORT_H__

#ifdef _WIN32
    #ifdef CSJRUNTIME_EXPORTS
        #define CSJRUNTIME_API __declspec(dllexport)
    #else 
        #define CSJRUNTIME_API __declspec(dllimport)
    #endif
#else // macOS and other Unix-like system.
    #ifdef CSJRUNTIME_EXPORTS
        #define CSJRUNTIME_API __attribute__((visibility("default")))
    #else
        #define CSJRUNTIME_API
    #endif
#endif // _WIN32

#endif // __CSJRUNTIME_EXPORTS_H__