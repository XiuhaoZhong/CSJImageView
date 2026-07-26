cmake_minimum_required(VERSION 3.16)

set(TARGET_NAME CSJVulkanRenderer)

project(${TARGET_NAME} LANGUAGES CXX)

# if(WIN32)
# set(Vulkan_DIR $ENV{VULKAN_SDK})
# set(Vulkan_Lib_DIR ${VulkanDIR}/Lib/cmake)
# set(vulkan_lib ${Vulkan_DIR}/Lib/vulkan-1.lib)
# elseif(APPLE)
# set(Vulkan_DIR "/Users/xiuhaozhong/DevEnvironments/VulkanSDK/1.4.304.0/macOS")
# set(vulkan_lib ${Vulkan_DIR}/Lib/libvulkan.1.dylib)
# endif()

find_package(Vulkan REQUIRED)

if(Vulkan_FOUND)
    message(STATUS "Vulkan found: ${Vulkan_VERSION}")
    message(STATUS "Vulkan include dir: ${Vulkan_INCLUDE_DIRS}")
    message(STATUS "Vulkan library: ${Vulkan_LIBRARIES}")
else()
    message(STATUS "Vulkan hasn't been found!")
endif()

set(VULKAN_SRC 
    CSJVulkanRenderer.cc
    ${CSJRHI_ROOT_DIR}/Public/CSJRendererFactory.cc
    )

add_library(${TARGET_NAME} SHARED
            ${VULKAN_SRC})

target_compile_definitions(${TARGET_NAME} PRIVATE CSJRHI_EXPORTS)

message(STATUS "ICSJRenderer path = ${CSJRHI_ROOT_DIR}/Public")

target_include_directories(${TARGET_NAME} PRIVATE
    ${CSJIMAGEVIEW_ROOT_DIR}/Utils 
    ${THIRD_PARTY_DIR}/glfw3/include/ 
    ${THIRD_PARTY_DIR}/stbi/ 
    ${Vulkan_DIR}/Include
    ${CSJRHI_ROOT_DIR}/Public
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${TARGET_NAME} PRIVATE 
                      Vulkan::Vulkan
                      glfw
                      ${CMAKE_DL_LIBS})

set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    FOLDER "CSJRHI/Backends"
)