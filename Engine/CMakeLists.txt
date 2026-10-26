cmake_minimum_required(VERSION 3.16)

set(TARGET_NAME "CSJEngineEditor")

add_definitions("-DUNICODE" "-D_UNICODE")

set(CSJENGINE_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(THIRD_PARTY_DIR "${CSJENGINE_ROOT_DIR}/3rdParts")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
    Editor/main/main.cc
    Editor/main/CSJEditor.h
    Editor/main/CSJEditor.cc
)

if(CMAKE_HOST_WIN32)
    set(Native_LIBS shlwapi)
else()
    set(Native_LIBS )
endif()

add_executable(${TARGET_NAME}
    ${PROJECT_SOURCES}
)

target_link_libraries(${TARGET_NAME} PRIVATE ${Native_LIBS})

if(APPLE)
    # macOS：生成普通可执行文件
    set_target_properties(${TARGET_NAME} PROPERTIES
        MACOSX_BUNDLE FALSE
    )

    # set_target_properties(${TARGET_NAME} PROPERTIES
    # ${BUNDLE_ID_OPTION}
    # MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    # MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    # MACOSX_BUNDLE TRUE
    # WIN32_EXECUTABLE TRUE
    # )
elseif(WIN32)
    # Windows：不弹控制台
    set_target_properties(${TARGET_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

add_subdirectory(${THIRD_PARTY_DIR})
add_subdirectory(CSJUtils)
add_subdirectory(Runtime)
add_subdirectory(CSJUILayer)

target_link_libraries(${TARGET_NAME} PRIVATE CSJUILayer CSJUtils CSJRuntime)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJ_ROOT_DIR}/resources"
        "$<TARGET_FILE_DIR:${TARGET_NAME}>/resources"
    COMMENT "Copying assets to output directory"
)

install(TARGETS ${TARGET_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
)