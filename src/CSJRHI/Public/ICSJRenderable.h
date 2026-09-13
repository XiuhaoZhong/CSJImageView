#pragma once

#include <cstdint>
#include <memory>

namespace csjrhi {
// ──────────────────────────────────────────────
// Effect Types
// ──────────────────────────────────────────────
enum class CSJPostProcessEffect {
    None = 0,           // Pass‑through
    Tonemap,        // Reinhard tonemapping + gamma
    Grayscale,      // Convert to grayscale
    Invert,         // Invert colors
    Sepia,          // Sepia tone
    Blur,           // Simple blur (placeholder)
    Bloom,          // Bloom (placeholder)
    Effect_Max,
};

struct CSJEffectParams {
    int effectType;   // 0 = None, 1 = Tonemap, 2 = Grayscale, 3 = Invert, 4 = Sepia
    float intensity;  // Optional: for future effects
    float exposure;   // Optional: for tonemapping
    float padding;    // used to align to 16-byte.
};


/**
 * @brief This interface is for the objects that manager their own rendering resouce and 
 *        are responsible for their rendering.
 */
class ICSJRenderable {
public:
    virtual ~ICSJRenderable() = default;

    virtual bool init(void* rendererHanle) = 0;
    virtual bool isReady() const = 0;
    virtual void updateScene() = 0;
    virtual void render(void* commandHandle, float timeStamp) = 0;
    virtual void onResize(uint32_t width, uint32_t height) = 0;
    virtual void unInit() = 0;

    virtual const char* GatName() const = 0;
};

using CSJSpRenderable = std::shared_ptr<ICSJRenderable>;
}