// RHI/RendererFactory.cpp
// Description: Implementation of renderer factory functions.
// Note: Uses conditional compilation to select the appropriate backend.

#include "CSJRendererFactory.h"

#include <cstdlib>
#include <iostream>

// ============================================================
// 平台检测宏（用于选择后端）
// ============================================================
#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
    #define PLATFORM_MACOS 1
#elif defined(__linux__)
    #define PLATFORM_LINUX 1
#else
    #error "Unsupported platform"
#endif

// ============================================================
// 条件编译：包含具体的后端实现头文件
// ============================================================
// 注意：这里只包含头文件，具体实现链接不同的动态库
// 或者直接包含后端的 .cpp 文件（如果是静态链接）

#ifdef PLATFORM_WINDOWS
    // Windows 上默认使用 Vulkan，也可以切换为 DX11
    #include "../Backends/Vulkan/CSJVulkanRenderer.h"
    // #include "Backends/DX11/DX11Renderer.h"  // 如果需要 DX11
#elif PLATFORM_MACOS
    //#include "Backends/Metal/MetalRenderer.h"
#elif PLATFORM_LINUX
    //#include "Backends/Vulkan/VulkanRenderer.h"
#endif

// ============================================================
// 命名空间
// ============================================================
using namespace csjrhi;

// ============================================================
// 工厂函数实现
// ============================================================
extern "C" {

/**
 * @brief Create a renderer instance based on the current platform.
 * 
 * This function selects the appropriate backend at compile time.
 * Future enhancements could use runtime selection via configuration.
 */
CSJRHI_API ICSJRenderer* CreateRenderer() {
    ICSJRenderer* renderer = nullptr;

#if defined(PLATFORM_WINDOWS)
    // Windows 平台：优先使用 Vulkan
    // 可以在这里添加环境变量或配置来决定使用哪个后端
    const char* forceBackend = "vulkan";//std::getenv("MINI_RENDERER_BACKEND");
    
    if (forceBackend) {
        std::string backend(forceBackend);
        if (backend == "DX11" || backend == "dx11") {
            // 未来可支持 DX11
            // renderer = new DX11Renderer();
            std::cerr << "[CSJRendererFactory] DX11 backend not yet implemented." << std::endl;
            return nullptr;
        } else if (backend == "Vulkan" || backend == "vulkan") {
            renderer = new CSJVulkanRenderer();
        } else {
            std::cerr << "[CSJRendererFactory] Unknown backend: " << backend << std::endl;
            return nullptr;
        }
    } else {
        // 默认使用 Vulkan
        renderer = new CSJVulkanRenderer();
    }
    
#elif defined(PLATFORM_MACOS)
    // macOS 平台：使用 Metal
    // renderer = new MetalRenderer();
    
#elif defined(PLATFORM_LINUX)
    // Linux 平台：使用 Vulkan
    // renderer = new VulkanRenderer();
    
#else
    #error "Unsupported platform"
#endif

    if (!renderer) {
        std::cerr << "[CSJRendererFactory] Failed to create renderer instance!" << std::endl;
    }

    return renderer;
}

/**
 * @brief Destroy a renderer instance and release all resources.
 */
CSJRHI_API void DestroyRenderer(ICSJRenderer* renderer) {
    if (renderer) {
        delete renderer;
        renderer = nullptr;
    }
}

/**
 * @brief Get the encoded version number of the RHI library.
 */
CSJRHI_API uint32_t GetRendererVersion() {
    return CSJRHI_VERSION_ENCODED;
}

/**
 * @brief Get the backend name as a static string.
 */
CSJRHI_API const char* GetBackendName() {
#if defined(PLATFORM_WINDOWS)
    const char* forceBackend = std::getenv("MINI_RENDERER_BACKEND");
    if (forceBackend) {
        std::string backend(forceBackend);
        if (backend == "DX11" || backend == "dx11") {
            return "DirectX 11";
        }
    }
    return "Vulkan";
#elif defined(PLATFORM_MACOS)
    return "Metal";
#elif defined(PLATFORM_LINUX)
    return "Vulkan";
#else
    return "Unknown";
#endif
}

/**
 * @brief Enable or disable debug mode (e.g., validation layers).
 */
CSJRHI_API void SetDebugMode(bool enable) {
    // 这个函数需要与具体的后端交互
    // 由于我们无法在此处直接访问后端的实例（还没有创建），
    // 可以设置一个全局标志，在 CreateRenderer 时传递给后端
    // 或者通过一个全局变量来控制
    
    static bool s_debugMode = false;
    s_debugMode = enable;
    
    std::cout << "[CSJRendererFactory] Debug mode " 
              << (enable ? "enabled" : "disabled") << std::endl;
}

} // extern "C"

// ============================================================
// 内部辅助函数（可选）
// ============================================================
namespace MinICSJRenderer {
namespace Internal {

/**
 * @brief Get the global debug mode flag.
 * 
 * This is used by backends to enable validation layers.
 */
bool IsDebugModeEnabled() {
    // 这里可以返回 SetDebugMode 设置的值
    // 但更合理的方式是让后端自己管理这个状态
    // 暂时简单返回 false
    return false;
}

} // namespace Internal
} // namespace MinICSJRenderer