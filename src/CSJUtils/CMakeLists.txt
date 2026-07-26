cmake_minimum_required(VERSION 3.15)

set(TARGET_NAME "CSJUtils")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INSTALL_BINDIR ${APP_OUTPUT_DIR})
set(CMAKE_INSTALL_LIBDIR ${GENERATE_LIB_DIR})

file(GLOB CSJUTIL_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB CSJUTIL_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc" )

# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/include
#     DESTINATION ${BUILD_DIR}/include/${TARGET_NAME}
#     FILES_MATCHING
#     PATTERN "*.h"
# )

add_custom_target(
    COPY_UTIL_HEADERS
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/include" "${BUILD_DIR}/include/${TARGET_NAME}"
)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if(APPLE)
    #find_package(lib-coreFoundation CoreFoundation)
endif()

add_library(${TARGET_NAME} SHARED ${CSJUTIL_SRC_FILES} ${CSJUTIL_HEADER_FILES})
add_dependencies(${TARGET_NAME} COPY_UTIL_HEADERS)

target_compile_definitions(${TARGET_NAME} PRIVATE CSJUTILS_EXPORTS)

target_link_libraries(${TARGET_NAME} PUBLIC spdlog::spdlog)
if(APPLE)
    target_link_libraries(${TARGET_NAME} PRIVATE "-framework CoreFoundation")
endif()

if(CMAKE_HOST_WIN32)
    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${TARGET_NAME}>" ${APP_OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE_DIR:${TARGET_NAME}>/$<TARGET_FILE_BASE_NAME:${TARGET_NAME}>.lib ${GENERATE_LIB_DIR} 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE_DIR:${TARGET_NAME}>/$<TARGET_FILE_BASE_NAME:${TARGET_NAME}>.pdb ${GENERATE_LIB_DIR} 
    )
elseif(CMAKE_HOST_APPLE)
    add_custom_command(
    TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${TARGET_NAME}>" ${GENERATE_LIB_DIR}
)
endif()