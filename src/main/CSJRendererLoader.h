// HostApp/CSJRendererLoader.h
// Description: Loads renderer backend DLL dynamically and manages function pointers.

#pragma once

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

#include <string>
#include <memory>

#include "ICSJRenderer.h"
#include "CSJRendererFactory.h"

/**
 * @brief Loads and manages a renderer backend dynamic library.
 * 
 * This class encapsulates the platform-specific dynamic loading logic
 * (LoadLibrary/dlopen) and provides a clean interface to create/destroy
 * renderer instances.
 */
class CSJRendererLoader {
public:
    CSJRendererLoader() = default;
    ~CSJRendererLoader() { Unload(); }

    // Delete copy constructor and assignment to avoid double-loading
    CSJRendererLoader(const CSJRendererLoader&) = delete;
    CSJRendererLoader& operator=(const CSJRendererLoader&) = delete;

    /**
     * @brief Load the renderer backend DLL.
     * 
     * @param backendName Name of the backend (e.g., "RendererVulkan", "RendererDX11").
     * @return true on success, false otherwise.
     */
    bool Load(const std::string& backendName);

    /**
     * @brief Unload the currently loaded DLL and reset function pointers.
     */
    void Unload();

    /**
     * @brief Create a renderer instance.
     * 
     * @return Pointer to IRenderer interface, or nullptr on failure.
     */
    csjrhi::ICSJRenderer* CreateRenderer() const;

    /**
     * @brief Destroy a renderer instance.
     * 
     * @param renderer Pointer returned by CreateRenderer().
     */
    void DestroyRenderer(csjrhi::ICSJRenderer* renderer) const;

    /**
     * @brief Get the backend name (e.g., "Vulkan", "DirectX 11").
     * 
     * @return C-style string, or "Unknown" if not loaded.
     */
    const char* GetBackendName() const;

    /**
     * @brief Get the RHI library version.
     * 
     * @return Encoded version number, or 0 if not loaded.
     */
    uint32_t GetVersion() const;

    /**
     * @brief Enable or disable debug mode.
     * 
     * @param enable true to enable validation layers, false to disable.
     */
    void SetDebugMode(bool enable) const;

    /**
     * @brief Check if a library is currently loaded.
     */
    bool IsLoaded() const { return m_handle != nullptr; }

private:
    // Private helper to get function pointer safely
    template <typename FuncPtr>
    FuncPtr GetProc(const char* name) const;

    // Platform-specific handle type
#ifdef _WIN32
    using HandleType = HMODULE;
#else
    using HandleType = void*;
#endif

    HandleType m_handle = nullptr;

    // Function pointers
    csjrhi::ICSJRenderer* (*m_createFunc)() = nullptr;
    void (*m_destroyFunc)(csjrhi::ICSJRenderer*) = nullptr;
    const char* (*m_nameFunc)() = nullptr;
    uint32_t (*m_versionFunc)() = nullptr;
    void (*m_debugFunc)(bool) = nullptr;
};

template <typename FuncPtr>
inline FuncPtr CSJRendererLoader::GetProc(const char *name) const {
    if (!m_handle) return nullptr;

#ifdef _WIN32
    return reinterpret_cast<FuncPtr>(GetProcAddress(m_handle, name));
#else
    return reinterpret_cast<FuncPtr>(dlsym(m_handle, name));
#endif
}
