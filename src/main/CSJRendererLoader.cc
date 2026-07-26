#include "CSJRendererLoader.h"

#include <iostream>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

using namespace csjrhi;

// ============================================================
// Auxiliary Function, get the error message.
// ============================================================
static std::string GetLastErrorString() {
#ifdef _WIN32
    DWORD error = GetLastError();
    LPWSTR buffer = nullptr;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   nullptr, error, 0, (LPWSTR)&buffer, 0, nullptr);
    
    std::string result;
    if (buffer) {
        int len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
        if (len > 0) {
            result.resize(len - 1);
            WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &result[0], len, nullptr, nullptr);
        }
        LocalFree(buffer);
    }
    return result.empty() ? "Unknown error" : result;
#else
    return dlerror() ? dlerror() : "Unknown error";
#endif
}

bool CSJRendererLoader::Load(const std::string &backendName) {
    // If the library has already been loaded, unload firstly.
    if (IsLoaded()) {
        Unload();
    }

    // 1. construct the complete file name.
#ifdef _WIN32

    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    fs::path currentPath = fs::path(buffer).parent_path();

    std::wstring wBackendName(backendName.begin(), backendName.end());
    std::wstring libraryPath = wBackendName + L".dll";
    currentPath = currentPath / libraryPath;
    if (fs::exists(currentPath)) {
        std::cout << "dll exists." << std::endl;
    } else {
        std::cout << "dll doesn't exist!" << std::endl;
    }
    std::wstring targetPath = currentPath.wstring();
    const wchar_t* path = targetPath.c_str();//libraryPath.c_str();
#else
    std::string libraryPath = "lib" + backendName + ".so";
    const char* path = libraryPath.c_str();
#endif

    std::cout << "[CSJRendererLoader] Loading backend: " << backendName << std::endl;

    // 2. Load the dynamic library.
#ifdef _WIN32
    m_handle = LoadLibraryW(path);
#else
    m_handle = dlopen(path, RTLD_NOW);
#endif

    if (!m_handle) {
        std::cerr << "[CSJRendererLoader] Failed to load library: " << backendName << std::endl;
        std::cerr << "[CSJRendererLoader] Error: " << GetLastErrorString() << std::endl;
        return false;
    }

    // 3. Get all the function pointers that will be used.
    m_createFunc   = GetProc<ICSJRenderer* (*)()>("CreateRenderer");
    m_destroyFunc  = GetProc<void (*)(ICSJRenderer*)>("DestroyRenderer");
    m_nameFunc     = GetProc<const char* (*)()>("GetBackendName");
    m_versionFunc  = GetProc<uint32_t (*)()>("GetRendererVersion");
    m_debugFunc    = GetProc<void (*)(bool)>("SetDebugMode");

    if (!m_createFunc || !m_destroyFunc) {
        std::cerr << "[CSJRendererLoader] Missing required factory functions!" << std::endl;
        Unload();
        return false;
    }

    std::cout << "[CSJRendererLoader] Loaded successfully." << std::endl;

    // Output the version information when it can be used.
    if (m_versionFunc) {
        uint32_t version = m_versionFunc();
        int major = (version >> 16) & 0xFF;
        int minor = (version >> 8) & 0xFF;
        int patch = version & 0xFF;
        std::cout << "[CSJRendererLoader] RHI version: " << major << "." << minor << "." << patch << std::endl;
    }

    return true;
}

void CSJRendererLoader::Unload() {
    if (m_handle) {
#ifdef _WIN32
        FreeLibrary(m_handle);
#else
        dlclose(m_handle);
#endif
        m_handle = nullptr;
    }

    m_createFunc  = nullptr;
    m_destroyFunc = nullptr;
    m_nameFunc    = nullptr;
    m_versionFunc = nullptr;
    m_debugFunc   = nullptr;

    std::cout << "[CSJRendererLoader] Unloaded." << std::endl;
}

ICSJRenderer *CSJRendererLoader::CreateRenderer() const {
    return m_createFunc ? m_createFunc() : nullptr;
}

void CSJRendererLoader::DestroyRenderer(ICSJRenderer *renderer) const {
     if (m_destroyFunc && renderer) {
        m_destroyFunc(renderer);
    }
}

const char *CSJRendererLoader::GetBackendName() const {
    return m_nameFunc ? m_nameFunc() : "Unknown";
}

uint32_t CSJRendererLoader::GetVersion() const {
    return m_versionFunc ? m_versionFunc() : 0;
}

void CSJRendererLoader::SetDebugMode(bool enable) const {
    if (m_debugFunc) {
        m_debugFunc(enable);
    } else {
        std::cout << "[CSJRendererLoader] SetDebugMode not available." << std::endl;
    }
}
