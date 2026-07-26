#pragma once 

#include <cstdint>
#include <string>

namespace csjrhi {

struct CSJRendererCapabilities;

class ICSJRenderer {
public:
    ICSJRenderer() = default;
    virtual ~ICSJRenderer() = default;

    /**
     * @brief Initialize renderer. 
     * 
     * @param windowHandle: platform window handle. (Windows: HWND, macOS: NSView*, Linux: XID)
     * @param width/height: original window size.
     * @return true success.
     */
    virtual bool Init(void* windowHandle, int width, int height) = 0;

    /**
     * @brief Close renderer，release all the GPU resources.
     */
    virtual void Shutdown() = 0;

    // Resize window.
    virtual void Resize(int width, int height) = 0;

    /**
     * @brief Rendering a frame. Inner workflow: Acquire -> Draw -> Present.
     */
    virtual void Render() = 0;

    /**
     * @brief Wait for GPU complete all the tasks, Useful for performance measurement
     *        or correctness validation.
     */
    virtual void WaitIdle() = 0;

    /** 
     * @brief Upload texture from CPU. Prepare for post process.
     * 
     * @param width Texture width in pixels.
     * @param height Texture height in pixels.
     * @param format format Pixel format identifier (interpreted by each backend).
     * @param data frame data pointer.
     * @return Opaque texture handle (backend-specific).
     */ 
    virtual uint32_t CreateTexture(int width, int height, int format, const void* data) = 0;

    /**
     * @brief Release a texture.
     * 
     * @param textureId Texture handle returned by CreateTexture().
     */
    virtual void DestroyTexture(uint32_t textureId) = 0;

    /** 
     * @brief Update texture for every rendering.
     * 
     * @param textureId Texture handle returned by CreateTexture().
     * @param data Pointer to new pixel data.
     */ 
    virtual void UpdateTexture(uint32_t textureId, const void* data) = 0;

    /**
     * @brief Get the backends name("Vulkan", "DirectX 11", "Metal", etc.), 
     *        for performance and debugging.
     * 
     * @return A static string literal (safe to pass across DLL boundaries).
     */
    virtual std::string GetBackendName() const = 0;

    /**
     * @brief Get current cost of GPU, in milliseconds.
     */
    virtual float GetLastFrameTime() const = 0;

    /**
     * @brief Get renderer capabilities.
     * 
     *  @return Reference to a RendererCapabilities struct.
     */
    virtual const CSJRendererCapabilities& GetCapabilities() const = 0;
};

// ============================================================
// Renderer capabilities descriptor
// ============================================================
struct CSJRendererCapabilities {
    bool supportsComputeShader;      /**< Whether compute shaders are supported */
    bool supportsTessellation;       /**< Whether tessellation is supported */
    bool supportsRayTracing;         /**< Whether ray tracing is supported */
    int  maxTextureSize;             /**< Maximum texture dimension in pixels */
    int  maxUniformBufferSize;       /**< Maximum uniform buffer size in bytes */
    const char* apiVersion;          /**< API version string (e.g., "1.3.250") */

    /** @brief Default constructor initializes all fields to safe defaults. */
    CSJRendererCapabilities()
        : supportsComputeShader(false)
        , supportsTessellation(false)
        , supportsRayTracing(false)
        , maxTextureSize(0)
        , maxUniformBufferSize(0)
        , apiVersion("Unknown") {}
};

} // namespace csjrhi