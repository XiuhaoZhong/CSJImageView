#pragma once 

#include "ICSJRenderer.h"

// ============================================================
// Cross-platform export/import macros
// ============================================================
#ifdef _WIN32
    #ifdef CSJRHI_EXPORTS
        #define CSJRHI_API __declspec(dllexport)
    #else
        #define CSJRHI_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    #ifdef CSJRHI_EXPORTS
        #define CSJRHI_API __attribute__((visibility("default")))
    #else
        #define CSJRHI_API
    #endif
#else
    #error "Unsupported platform"
#endif

// ============================================================
// Version information
// ============================================================
#define CSJRHI_VERSION_MAJOR 1   /**< Major version number */
#define CSJRHI_VERSION_MINOR 0   /**< Minor version number */
#define CSJRHI_VERSION_PATCH 0   /**< Patch version number */

/**
 * @brief Encode version numbers into a single 32-bit integer.
 * 
 * @return Version encoded as (MAJOR << 16) | (MINOR << 8) | PATCH.
 */
#define CSJRHI_VERSION_ENCODED \
    ((CSJRHI_VERSION_MAJOR << 16) | (CSJRHI_VERSION_MINOR << 8) | CSJRHI_VERSION_PATCH)

// ============================================================
// Factory functions (pure C interface for ABI stability)
// ============================================================
extern "C" {

    /**
     * @brief Create a renderer instance.
     * 
     * @return Pointer to ICSJRenderer interface, or nullptr on failure.
     */
    CSJRHI_API csjrhi::ICSJRenderer* CreateRenderer();

    /**
     * @brief Destroy a renderer instance and release all associated resources.
     * 
     * @param renderer Pointer returned by CreateRenderer().
     */
    CSJRHI_API void DestroyRenderer(csjrhi::ICSJRenderer* renderer);

    /**
     * @brief Get the encoded version number of the RHI library.
     * 
     * @return Version encoded as (MAJOR << 16) | (MINOR << 8) | PATCH.
     */
    CSJRHI_API uint32_t GetRendererVersion();

    /**
     * @brief Get the backend name as a static string.
     * 
     * @return C-style string literal (e.g., "Vulkan", "DirectX 11", "Metal").
     */
    CSJRHI_API const char* GetBackendName();

    /**
     * @brief Enable or disable debug mode (e.g., validation layers).
     * 
     * @param enable true to enable debug mode, false to disable.
     */
    CSJRHI_API void SetDebugMode(bool enable);
}

// ============================================================
// C++ helpers (optional, for RAII-style lifetime management)
// ============================================================
#ifdef __cplusplus
#include <memory>

namespace csjrhi {

/**
 * @brief Custom deleter for ICSJRenderer pointers.
 * 
 * Ensures DestroyRenderer() is called automatically when the unique_ptr goes out of scope.
 */
struct RendererDeleter {
    /**
     * @brief Call DestroyRenderer() on the given pointer.
     * 
     * @param renderer Pointer to ICSJRenderer instance.
     */
    void operator()(ICSJRenderer* renderer) const {
        if (renderer) {
            DestroyRenderer(renderer);
        }
    }
};

/**
 * @brief Unique pointer type for ICSJRenderer with automatic cleanup.
 */
using RendererPtr = std::unique_ptr<ICSJRenderer, RendererDeleter>;

/**
 * @brief Create a renderer and wrap it in a unique_ptr.
 * 
 * @return RendererPtr that will automatically call DestroyRenderer() on destruction.
 */
inline RendererPtr CreateRendererUnique() {
    return RendererPtr(CreateRenderer());
}

} // namespace MinICSJRenderer
#endif // __cp