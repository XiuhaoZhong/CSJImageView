cmake_minimum_required(VERSION 3.16)

set(TARGET_NAME CSJImageView)

project(${TARGET_NAME} VERSION 0.1 LANGUAGES CXX)

add_definitions("-DUNICODE" "-D_UNICODE")

set(CSJIMAGEVIEW_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(THIRD_PARTY_DIR "${CSJIMAGEVIEW_ROOT_DIR}/3rdParts")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
    src/main/main.cc
    src/main/CSJApplication.h
    src/main/CSJApplication.cc
    src/main/CSJRendererLoader.h
    src/main/CSJRendererLoader.cc
    src/Utils/CSJPathTool.h
    src/Utils/CSJPathTool.cc
)

message(STATUS "src files = ${PROJECT_SOURCES}")

include_directories(Utils 3rdParts/glfw3/include/ ${Vulkan_DIR}/Include)
set(glfw_include ${THIRD_PARTY_DIR}/glfw/include)

if(CMAKE_HOST_WIN32)
set(Native_LIBS shlwapi)
else()
set(Native_LIBS )
endif()

add_executable(${TARGET_NAME}
            ${PROJECT_SOURCES}
)

target_include_directories(${TARGET_NAME} PRIVATE
    ${CSJIMAGEVIEW_ROOT_DIR}/src/CSJRHI/Public
)

target_link_libraries(${TARGET_NAME} PRIVATE ${Native_LIBS})
target_link_libraries(${TARGET_NAME} PUBLIC glfw)

target_include_directories(${TARGET_NAME} 
                           PUBLIC $<BUILD_INTERFACE:${glfw_include}>
                           ${CSJIMAGEVIEW_ROOT_DIR}/src)

# get_target_property(TARGET_INCLUDES MyTarget INCLUDE_DIRECTORIES)
# message(STATUS "MyTarget include directories: ${TARGET_INCLUDES}")

set_target_properties(${TARGET_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CSJIMAGEVIEW_ROOT_DIR}/build/bin/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CSJIMAGEVIEW_ROOT_DIR}/build/bin/$<CONFIG>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CSJIMAGEVIEW_ROOT_DIR}/build/lib/$<CONFIG>)

add_subdirectory(${THIRD_PARTY_DIR})
add_subdirectory(src/CSJRHI)

include(GNUInstallDirs)
install(TARGETS CSJImageView
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
)
