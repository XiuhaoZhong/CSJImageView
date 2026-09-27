cmake_minimum_required(VERSION 3.16)

set(TARGET_NAME CSJUILayer)

find_package(Vulkan REQUIRED)

set(CSJUILAYER_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# create a static library inner UILayer, compile ImGui and UILayer.
add_library(${TARGET_NAME} STATIC
    CSJUILayer.cc

    # ImGui kernel
    ImGui/imgui.cpp
    ImGui/imgui_draw.cpp
    ImGui/imgui_tables.cpp
    ImGui/imgui_widgets.cpp

    # ImGui backend
    ImGui/backends/imgui_impl_glfw.cpp
    ImGui/backends/imgui_impl_vulkan.cpp
)
include_directories(Utils 3rdParts/glfw3/include/ ${Vulkan_DIR}/Include)
target_include_directories(${TARGET_NAME}
                           PUBLIC $<BUILD_INTERFACE:${glfw_include}>)

# Expose headers of UILayer and ImGu.
target_include_directories(${TARGET_NAME} PUBLIC
    ${CSJUILAYER_ROOT_DIR}
    ${CSJUILAYER_ROOT_DIR}/ImGui
    ${CSJUILAYER_ROOT_DIR}/ImGui/backends
)

# 链接后端依赖
target_link_libraries(${TARGET_NAME} PUBLIC
    glfw
    Vulkan::Vulkan
    CSJUtils
)