#ifndef __CSJLOGGER_H__
#define __CSJLOGGER_H__

#include "CSJUtils_Export.h"

#ifdef __cplusplus
extern "C" {
#endif

CSJUTILS_API void CSJLog_Init(const char* logFile);
CSJUTILS_API void CSJLog_Uninit();
CSJUTILS_API void CSJLog_Debug(const char* file, int line, const char* format, ...);
CSJUTILS_API void CSJLog_Info(const char* file, int line, const char* format, ...);
CSJUTILS_API void CSJLog_Warn(const char* file, int line, const char* format, ...);
CSJUTILS_API void CSJLog_Error(const char* file, int line, const char* format, ...);

#define LOG_Debug(...) CSJLog_Debug(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_Info(...) CSJLog_Debug(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_Warn(...) CSJLog_Debug(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_Error(...) CSJLog_Debug(__FILE__, __LINE__, __VA_ARGS__)


#ifdef __cplusplus
}
#endif

#endif // __CSJLOGGER_H__