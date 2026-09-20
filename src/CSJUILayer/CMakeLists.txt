cmake_minimum_required(VERSION 3.16)

set(TARGET_NAME CSJUILayer)

find_package(Vulkan REQUIRED)

set(CSJUILAYER_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# 在 UILayer 目录内创建一个静态库，编译 ImGui 源码 + UILayer 自身
add_library(${TARGET_NAME} STATIC
    CSJUILayer.cc

    # ImGui 核心
    ImGui/imgui.cpp
    ImGui/imgui_draw.cpp
    ImGui/imgui_tables.cpp
    ImGui/imgui_widgets.cpp

    # ImGui 后端
    ImGui/backends/imgui_impl_glfw.cpp
    ImGui/backends/imgui_impl_vulkan.cpp
)
include_directories(Utils 3rdParts/glfw3/include/ ${Vulkan_DIR}/Include)
target_include_directories(${TARGET_NAME}
                           PUBLIC $<BUILD_INTERFACE:${glfw_include}>)

# 公开 UILayer 自身的头文件，以及 ImGui 的头文件路径
target_include_directories(${TARGET_NAME} PUBLIC
    ${CSJUILAYER_ROOT_DIR}
    ${CSJUILAYER_ROOT_DIR}/ImGui
    ${CSJUILAYER_ROOT_DIR}/ImGui/backends
)

# 链接后端依赖
target_link_libraries(${TARGET_NAME} PUBLIC
    glfw
    Vulkan::Vulkan
)