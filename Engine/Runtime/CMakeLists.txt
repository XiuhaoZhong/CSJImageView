cmake_minimum_required(VERSION 3.16)

set(TARGET_NAME "CSJRuntime")

set(RUNTIME_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Vulkan REQUIRED)

if(Vulkan_FOUND)
    message(STATUS "Runtime Vulkan found: ${Vulkan_VERSION}")
    message(STATUS "Vulkan include dir: ${Vulkan_INCLUDE_DIRS}")
    message(STATUS "Vulkan library: ${Vulkan_LIBRARIES}")
else()
    message(STATUS "Vulkan hasn't been found!")
endif()

file(GLOB Window_UI_Source_files CONFIGURE_DEPENDS
    # ImGui kernel
    render/CSJWindowUI/ImGui/imgui.cpp
    render/CSJWindowUI/ImGui/imgui_draw.cpp
    render/CSJWindowUI/ImGui/imgui_tables.cpp
    render/CSJWindowUI/ImGui/imgui_widgets.cpp

    # ImGui backend
    render/CSJWindowUI/ImGui/backends/imgui_impl_glfw.cpp
    render/CSJWindowUI/ImGui/backends/imgui_impl_vulkan.cpp    
)

file(GLOB Runtime_Header_Files CONFIGURE_DEPENDS
    "include/*.h" 
    "render/*.h"
    "global/*.h"
    "render/CSJWindowUI/*.h"
)

file(GLOB Runtime_Source_Files CONFIGURE_DEPENDS 
    "source/*.cc" 
    "render/*.cc"
    "global/*.cc"
    "render/CSJWindowUI/*.cc"
)

add_library(${TARGET_NAME} SHARED 
            ${Runtime_Header_Files}
            ${Runtime_Source_Files} 
            ${Window_UI_Source_files}
)

target_compile_definitions(${TARGET_NAME} PRIVATE CSJRUNTIME_EXPORTS)

target_include_directories(${TARGET_NAME} PUBLIC 
    ${RUNTIME_ROOT_DIR}/include
    ${RUNTIME_ROOT_DIR}/render
    ${RUNTIME_ROOT_DIR}/global
    ${RUNTIME_ROOT_DIR}/render/CSJWindowUI/ImGui/
    ${RUNTIME_ROOT_DIR}/render/CSJWindowUI/ImGui/backends
)

target_include_directories(${TARGET_NAME} PUBLIC
    ${RUNTIME_ROOT_DIR}/render/CSJRHI/Public
)

target_link_libraries(${TARGET_NAME} PUBLIC 
                      glfw
                      CSJUtils
)

target_link_libraries(${TARGET_NAME} PUBLIC RHI_Interfaces Vulkan::Vulkan)

set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    FOLDER "Engine/Runtime"
)

add_subdirectory(render/CSJRHI)