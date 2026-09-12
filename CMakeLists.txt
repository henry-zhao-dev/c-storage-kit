cmake_minimum_required(VERSION 3.21)

project(cstorage_kit
    VERSION 0.1.0
    DESCRIPTION "A lightweight C library for generic storage abstractions"
    LANGUAGES C
)

include(GNUInstallDirs)

option(CSTORAGE_KIT_BUILD_EXAMPLES "Build the cstorage-kit examples" OFF)
option(CSTORAGE_KIT_ENABLE_SANITIZERS
    "Build with AddressSanitizer and UndefinedBehaviorSanitizer"
    OFF
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CSTORAGE_KIT_SOURCES
    src/calist.c
    src/ctype.c
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

if(CSTORAGE_KIT_BUILD_EXAMPLES)
    add_executable(cstorage_kit_demo examples/demo_calist_int.c)
    target_link_libraries(cstorage_kit_demo PRIVATE CStorageKit::Core)
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
