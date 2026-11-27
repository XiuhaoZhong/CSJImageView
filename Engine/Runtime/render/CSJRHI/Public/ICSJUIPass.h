#pragma once 

#include "CSJRenderContextDefinition.h"

namespace csjrhi {

class ICSJUIPass {
public:
    virtual void render(void* commandBuffer) = 0;
    virtual void shutdown() = 0;
};

} // namespace csjrhi