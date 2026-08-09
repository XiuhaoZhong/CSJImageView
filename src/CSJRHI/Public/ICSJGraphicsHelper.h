#pragma once 

#include <memory>
#include <string>
#include <functional>

namespace csjrhi {
// ============================================================
// Enums
// ============================================================

enum class CSJPixelFormat : uint32_t {
    CSJPixelFormat_NONE = 0,
    CSJPixelFormat_R8G8B8A8_SRGB,
    CSJPixelFormat_R8G8B8A8_UNORM,
    CSJPixelFormat_R8G8B8_UNORM,
    CSJPixelFormat_R8_UNORM,
    CSJPixelFormat_R16G16B16A16_SFLOAT,
    CSJPixelFormat_R16G16B16_SFLOAT,
    CSJPixelFormat_R32G32B32A32_SFLOAT,
    CSJPixelFormat_R32G32B32_SFLOAT,
    CSJPixelFormat_D32_SFLOAT_S8_UINT,
    CSJPixelFormat_D32_SFLOAT,
    CSJPixelFormat_D24_UNORM_S8_UINT,
    CSJPixelFormat_D16_UNORM,
    CSJPixelFormat_YUV420_PLANAR,
    CSJPixelFormat_YUV420_SEMIPLANAR,
    CSJPixelFormat_NV12,
    CSJPixelFormat_NV21,
    CSJPixelFormat_BC1_RGB_UNORM,
    CSJPixelFormat_BC1_RGBA_UNORM,
    CSJPixelFormat_BC3_UNORM,
    CSJPixelFormat_BC7_UNORM,
    CSJPixelFormat_UNDEFINED
};

enum class CSJBufferUsage : uint32_t {
    CSJBufferUsage_None = 0,
    CSJBufferUsage_Vertex   = 1 << 0,
    CSJBufferUsage_Index    = 1 << 1,
    CSJBufferUsage_Uniform  = 1 << 2,
    CSJBufferUsage_Storage  = 1 << 3,
    CSJBufferUsage_Staging  = 1 << 4,
    CSJBufferUsage_Transfer = 1 << 5,
    CSJBufferUsage_Indirect = 1 << 6,
};

// ============================================================
// Abstract Interface
// ============================================================

class ICSJTexture {
public:
    virtual ~ICSJTexture() = default;
    virtual void* GetNativeHandle() = 0;
    virtual void* GetSampler() = 0;
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual uint32_t GetMipLevels() const = 0;
    virtual CSJPixelFormat GetFormat() const = 0;
};

class ICSJBuffer {
public:
    virtual ~ICSJBuffer() = default;
    virtual void* GetNativeHandle() = 0;
    virtual size_t GetSize() const = 0;
    virtual CSJBufferUsage GetUsage() const = 0;
};

using CSJSpTexture = std::unique_ptr<ICSJTexture>;
using CSJSpBuffer = std::unique_ptr<ICSJBuffer>;

// ============================================================
// Main Toolkit Interface
// ============================================================

class ICSJGraphicsHelper {
public:
    virtual ~ICSJGraphicsHelper() = default;

    // --- Texture ---
    virtual CSJSpTexture CreateTexture2D(uint32_t width,
                                         uint32_t height,
                                         CSJPixelFormat format,           // Abstract format (RGBA8, YUV, etc.)
                                         const void* data,
                                         size_t dataSize,
                                         bool generateMipmaps = false) = 0;

    virtual CSJSpTexture CreateTextureFromFile(const std::string& filePath) = 0;

    virtual void UpdateTexture(ICSJTexture* texture,
                               const void* data,
                               size_t dataSize) = 0;

    virtual void DestroyTexture(ICSJTexture* texture) = 0;

    // --- Buffer ---
    virtual CSJSpBuffer CreateBuffer(size_t size,
                                     CSJBufferUsage usage,            // Vertex, Index, Uniform, Staging
                                     const void* data = nullptr) = 0;

    virtual void UpdateBuffer(ICSJBuffer* buffer,
                              const void* data,
                              size_t dataSize,
                              size_t offset = 0) = 0;

    virtual void DestroyBuffer(ICSJBuffer* buffer) = 0;

    // --- Command Buffer (Helper) ---
    virtual void ExecuteImmediate(
        const std::function<void(void*)>& commands
    ) = 0;  // For single-time commands (e.g., upload data)

    // --- Info ---
    virtual std::string GetBackendName() const = 0;
    virtual void* GetDeviceHandle() const = 0;
};

} // namespace csjrhi 