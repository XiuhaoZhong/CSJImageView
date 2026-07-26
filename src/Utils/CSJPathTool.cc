#include "CSJPathTool.h"

#if defined(WIN32)

#include <windows.h>
#include <shlwapi.h>

static char* wchar2char(LPWSTR tcStr); 
static std::string getCurRunPath();

#elif defined(__APPLE__)
#endif

std::string CSJPathTool::getExecutePath() {
#if defined(WIN32)
    return getCurRunPath();
#elif defined(__APPLE__)
#endif
    return std::string();
}

#if defined(WIN32) 
static char* wchar2char(LPWSTR tcStr) {
    if (!tcStr) {
        return NULL;
    }

    char *directory = new char[255];
    int length = wcslen(tcStr);

    int outputLength = WideCharToMultiByte(CP_ACP, 0, tcStr, length, NULL, 0, 0, 0) + 2;
    memset(directory, 0x00, 255);
    WideCharToMultiByte(CP_ACP, 0, tcStr, length, directory, outputLength, 0, 0);

    return directory;
}

static TCHAR* char2wchar(const char* cStr) {
    if (!cStr) {
        return NULL;
    }

    int w_nlen = MultiByteToWideChar(CP_ACP, 0, cStr, -1, NULL, 0);
    WCHAR *ret;
    ret = (WCHAR*)malloc(sizeof(WCHAR)*w_nlen);
    memset(ret, 0, sizeof(ret));
    MultiByteToWideChar(CP_ACP, 0, cStr, -1, ret, w_nlen);
    return ret;
}

std::string getCurRunPath() {
    TCHAR szPath[255];
    ::GetModuleFileName(NULL, szPath, MAX_PATH);
    char *path = wchar2char(szPath);

    TCHAR curPath[255];
    DWORD pathLen = ::GetCurrentDirectory(255, curPath);
    if (pathLen > 0) {

    }

    path = wchar2char(curPath);
    TCHAR szCurDir[MAX_PATH];
    DWORD dirLen = ::GetFullPathName(TEXT("C:"), MAX_PATH, szCurDir, NULL);

    std::string strPath(path);

    delete path;
    return strPath;
}

// _CRT_SECURE_NO_WARNINGS
std::string getResourcePath(std::string relativePath) {
    TCHAR curPath[255];
    DWORD pathLen = ::GetCurrentDirectory(255, curPath);
    if (pathLen > 0) {

    }

    char *cstr = new char[relativePath.length() + 1];
    std::strcpy(cstr, relativePath.c_str());

    ::PathAppend(curPath, char2wchar(cstr));

    char *path = wchar2char(curPath);
    path = wchar2char(curPath);

    std::string resPath(path);

    delete path;
    return resPath;
}
#elif defined(__APPLE__)
#endif