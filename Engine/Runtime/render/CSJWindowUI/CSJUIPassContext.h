#pragma once

#include <cstdint>

namespace csjruntime {

enum class CSJBackendType {
    Vulkan,
    DX11,
    DX12,
    Metal
};

} // namespace csjuilayer