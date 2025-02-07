cmake_minimum_required(VERSION 3.16)

set(TARGET_NAME CSJImageView)

project(${TARGET_NAME} VERSION 0.1 LANGUAGES CXX)

add_definitions("-DUNICODE" "-D_UNICODE")

add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

set(CSJIMAGEVIEW_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(THIRD_PARTY_DIR "${CSJIMAGEVIEW_ROOT_DIR}/3rdParts")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
        main/main.cc
        main/CSJApplication.h
        main/CSJApplication.cc
        Utils/CSJPathTool.h
        Utils/CSJPathTool.cc
)

if(WIN32)
set(Vulkan_DIR $ENV{VULKAN_SDK})
set(Vulkan_Lib_DIR ${VulkanDIR}/Lib/cmake)
set(vulkan_lib ${Vulkan_DIR}/Lib/vulkan-1.lib)
elseif(APPLE)
set(Vulkan_DIR "/Users/xiuhaozhong/DevEnvironments/VulkanSDK/1.4.304.0/macOS")
set(vulkan_lib ${Vulkan_DIR}/Lib/libvulkan.1.dylib)
endif()
include_directories(Utils 3rdParts/glfw3/include/ 3rdParts/stbi/ ${Vulkan_DIR}/Include)
set(glfw_include ${THIRD_PARTY_DIR}/glfw/include)

if(CMAKE_HOST_WIN32)
set(Native_LIBS shlwapi)
else()
set(Native_LIBS )
endif()

add_executable(${TARGET_NAME}
            ${PROJECT_SOURCES}
)

target_link_libraries(${TARGET_NAME} PRIVATE ${vulkan_lib} ${Native_LIBS})
target_link_libraries(${TARGET_NAME} PUBLIC glfw)

target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${glfw_include}>)

set_target_properties(${TARGET_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

add_subdirectory(${THIRD_PARTY_DIR})

include(GNUInstallDirs)
install(TARGETS CSJImageView
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
