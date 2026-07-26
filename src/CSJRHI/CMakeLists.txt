cmake_minimum_required(VERSION 3.16)


set(CSJRHI_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(${CSJRHI_ROOT_DIR}/Backends/Vulkan)

if(WIN32)
# add_subdirectory(${CSJRHI_ROOT_DIR}/Backends/DirectX)
endif()

if(APPLE)
add_subdirectory(${CSJRHI_ROOT_DIR}/Backends/Metal)
endif()

add_library(RHI_Interfaces INTERFACE)
target_include_directories(RHI_Interfaces INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/Public
)