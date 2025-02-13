#include "common.hpp"
#include <string>
#include <cmath>
#include <climits>
#include <limits>

#if defined( __APPLE_CC__ ) || defined ( __APPLE__ )
	#define PLATFORM_OSX
	#include <CoreFoundation/CoreFoundation.h>
#elif defined( __WIN32__ ) || defined( _WIN32 ) || defined( _WIN64 )
	#define PLATFORM_WIN32
	#include <windows.h>
#elif defined ( linux ) || defined( __linux__ )
	#define PLATFORM_LINUX
	#include <libgen.h>
	#include <unistd.h>
#elif defined( __HAIKU__ ) || defined( __BEOS__ )
	#define PLATFORM_HAIKU
	#include <kernel/OS.h>
	#include <kernel/image.h>
#elif defined( __SVR4 )
	#define PLATFORM_SOLARIS
	#include <stdlib.h>
#elif defined( __FreeBSD__ ) || defined(__OpenBSD__) || defined( __NetBSD__ ) || defined( __DragonFly__ )
	#define PLATFORM_BSD
	#include <sys/sysctl.h>
#endif

std::string WChar2Ansi(LPCWSTR pwszSrc) {
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);

	if (nLen <= 0) {
		return std::string("");
	}

	char * pszDst = new char[nLen];
	if (pszDst == NULL) {
		return std::string("");
	}

	WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
	pszDst[nLen - 1] = 0;

	std::string strTemp(pszDst);
	delete []pszDst;

	return strTemp;
}

std::string ws2s(std::wstring& inputws) {
	return WChar2Ansi(inputws.c_str());
}

std::string FileRemoveFileName( const std::string& filepath ) {
	return filepath.substr( 0, filepath.find_last_of("/\\") + 1 );
}

static std::string GetProcessPath() {
#if defined( PLATFORM_OSX )
	char exe_file[PATH_MAX + 1];
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	if (mainBundle) {
		CFURLRef mainURL = CFBundleCopyBundleURL(mainBundle);

		if (mainURL) {
			int ok = CFURLGetFileSystemRepresentation ( mainURL, (Boolean) true, (UInt8*)exe_file, PATH_MAX );

			if (ok) {
				return std::string(exe_file) + "/";
			}
		}
	}

	return "./";
#elif defined( PLATFORM_LINUX )
	char exe_file[PATH_MAX + 1];
	int size;
	size = readlink("/proc/self/exe", exe_file, PATH_MAX);
	if (size < 0) {
		return "./";
	} else {
		exe_file[size] = '\0';
		return std::string(dirname(exe_file)) + "/";
	}
#elif defined( PLATFORM_WIN32 )
	// Get path to executable:
	TCHAR szDllName[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFilename[_MAX_DIR];
	TCHAR szExt[_MAX_DIR];
	GetModuleFileName(0, szDllName, _MAX_PATH);
#ifdef  UNICODE
	_wsplitpath(szDllName, szDrive, szDir, szFilename, szExt);
	std::wstring finalPath = std::wstring(szDrive) + std::wstring(szDir);
	return ws2s(finalPath);
#else
	_splitpath(szDllName, szDrive, szDir, szFilename, szExt);
	return std::string(szDrive) + std::string(szDir);
#endif
	
#elif defined( PLATFORM_BSD )
	int mib[4];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PATHNAME;
	mib[3] = -1;
	char buf[1024];
	size_t cb = sizeof(buf);
	sysctl(mib, 4, buf, &cb, NULL, 0);

	return FileRemoveFileName( std::string( buf ) );
#elif defined( PLATFORM_SOLARIS )
	return FileRemoveFileName( std::string( getexecname() ) );
#elif defined( PLATFORM_HAIKU )
	image_info info;
	int32 cookie = 0;

	while ( B_OK == get_next_image_info( 0, &cookie, &info ) ) {
		if ( info.type == B_APP_IMAGE )
			break;
	}

	return FileRemoveFileName( std::string( info.name ) );
#else
	#warning GetProcessPath() not implemented on this platform. ( will return "./" )
	return "./";
#endif
}

std::string ResourcePath(std::string fileName) {
	return GetProcessPath() + fileName;
}

