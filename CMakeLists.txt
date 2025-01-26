cmake_minimum_required(VERSION 3.16)

project(CSJImageView VERSION 0.1 LANGUAGES CXX)

add_definitions("-DUNICODE" "-D_UNICODE")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
        main/main.cc
        main/CSJApplication.h
        main/CSJApplication.cc
        Utils/CSJPathTool.h
        Utils/CSJPathTool.cc
)

set(Vulkan_DIR $ENV{VULKAN_SDK})
set(Vulkan_Lib_DIR ${VulkanDIR}/Lib/cmake)
set(vulkan_lib ${Vulkan_DIR}/Lib/vulkan-1.lib)
#find_package(Vulkan REQUIRED)

include_directories(Utils 3rdParts/glfw3/include/ 3rdParts/stbi/ ${Vulkan_DIR}/Include)

link_directories(3rdParts/glfw3/lib/)

if(CMAKE_HOST_WIN32)
set(Native_LIBS shlwapi)
else()
set(Native_LIBS shlwapi)
endif()

add_executable(CSJImageView
            ${PROJECT_SOURCES}
)

target_link_libraries(CSJImageView PRIVATE glfw3 ${vulkan_lib} ${Native_LIBS})

set_target_properties(CSJImageView PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS CSJImageView
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
