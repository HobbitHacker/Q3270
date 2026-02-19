cmake_minimum_required(VERSION 3.16)

project(Q3270 VERSION 0.2.0 LANGUAGES CXX)

# --- NEW: Target Apple Silicon (M4/M-series) ---
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
endif()

# Append Git hash to build to track commits
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(Q3270_VERSION_FULL "${PROJECT_VERSION}+g${GIT_HASH}")

# Configure version header
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/Version.h
    @ONLY
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt meta compilers
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Default to Release if nothing specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# Find Qt (Qt5 or Qt6)
find_package(Qt6 QUIET COMPONENTS Core Gui Widgets Network Svg SvgWidgets)
if(NOT Qt6_FOUND)
  find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Network Svg)
endif()

add_compile_options(-Wall -Wextra -Wunused-function -fdiagnostics-show-option -fno-diagnostics-color)

add_definitions(-DQT_DEPRECATED_WARNINGS)

# build logic for code in src/
add_subdirectory(src)

if(APPLE)
    # This tells CMake to put the .app in the root of the install directory
    set_target_properties(Q3270 PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Q3270 Emulator"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.hobbithacker.q3270"
    )

    install(TARGETS Q3270
        BUNDLE DESTINATION .
        RUNTIME DESTINATION bin
    )
else()
    # Your existing Linux/Debian install logic
    install(TARGETS Q3270 RUNTIME DESTINATION bin)
    option(DEBIAN_INSTALL "Install paths for Debian packaging" OFF)
    if(DEBIAN_INSTALL)
        install(FILES LICENSE README.md DESTINATION share/q3270)
    else()
        install(FILES LICENSE README.md DESTINATION share/Q3270)
    endif()
endif()

option(DEBIAN_INSTALL "Install paths for Debian packaging" OFF)

if(DEBIAN_INSTALL)
    install(FILES LICENSE README.md DESTINATION share/q3270)
else()
    install(FILES LICENSE README.md DESTINATION share/Q3270)
endif()
