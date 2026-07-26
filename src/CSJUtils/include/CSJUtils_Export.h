#ifndef __CSJUTILS_EXPORT_H__
#define __CSJUTILS_EXPORT_H__

#ifdef _WIN32
    #ifdef CSJUTILS_EXPORTS
        #define CSJUTILS_API __declspec(dllexport)
    #else 
        #define CSJUTILS_API __declspec(dllimport)
    #endif
#else // macOS and other Unix-like system.
    #ifdef CSJUTILS_EXPORTS
        #define CSJUTILS_API __attribute__((visibility("default")))
    #else
        #define CSJUTILS_API
    #endif
#endif // _WIN32

#endif // __CSJMEDIAEGNINE_EXPORT_H__