cmake_minimum_required(VERSION 3.21)

project(cstorage_kit
    VERSION 0.1.0
    DESCRIPTION "A lightweight C library for generic storage abstractions"
    LANGUAGES C
)

include(GNUInstallDirs)

set(CSTORAGE_KIT_STANDALONE OFF)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(CSTORAGE_KIT_STANDALONE ON)
endif()

option(CSTORAGE_KIT_BUILD_TESTS
    "Build the cstorage-kit unit tests"
    ${CSTORAGE_KIT_STANDALONE}
)
option(CSTORAGE_KIT_ENABLE_SANITIZERS
    "Build with AddressSanitizer and UndefinedBehaviorSanitizer"
    OFF
)

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CSTORAGE_KIT_SOURCES
    src/calist.c
    src/cvalue.c
)

add_library(cstorage_kit STATIC ${CSTORAGE_KIT_SOURCES})
add_library(CStorageKit::Core ALIAS cstorage_kit)

set_target_properties(cstorage_kit PROPERTIES
    EXPORT_NAME Core
)

target_include_directories(cstorage_kit
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_options(cstorage_kit PRIVATE
    $<$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>
    $<$<COMPILE_LANG_AND_ID:C,MSVC>:/W4>
)

if(CSTORAGE_KIT_ENABLE_SANITIZERS AND
   CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(cstorage_kit PUBLIC
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_options(cstorage_kit PUBLIC -fsanitize=address,undefined)
endif()

if(CSTORAGE_KIT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(CLANG_FORMAT_EXECUTABLE)
    file(GLOB_RECURSE CSTORAGE_KIT_FORMAT_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h"
    )

    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXECUTABLE} --style=file -i ${CSTORAGE_KIT_FORMAT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting c-storage-kit sources"
        VERBATIM
    )

    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT_EXECUTABLE} --style=file --dry-run --Werror
            ${CSTORAGE_KIT_FORMAT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Checking c-storage-kit formatting"
        VERBATIM
    )
else()
    message(STATUS "clang-format not found; format targets are unavailable")
endif()

install(TARGETS cstorage_kit
    EXPORT CStorageKitTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT CStorageKitTargets
    FILE CStorageKitTargets.cmake
    NAMESPACE CStorageKit::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CStorageKit
)
