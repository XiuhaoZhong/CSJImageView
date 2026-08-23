#pragma once

#include <cstdint>
#include <memory>

namespace csjrhi {

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