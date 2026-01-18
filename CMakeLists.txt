cmake_minimum_required(VERSION 3.16)

project(sound2osc
    VERSION 0.1.0
    DESCRIPTION "Audio-to-OSC trigger generator"
    LANGUAGES CXX
)

# -----------------------------------------------------------------------------
# Project options
# -----------------------------------------------------------------------------
option(SOUND2OSC_BUILD_GUI "Build the Qt GUI application" ON)
option(SOUND2OSC_BUILD_HEADLESS "Build the headless CLI application" OFF)
option(SOUND2OSC_BUILD_TESTS "Build unit tests" OFF)

# -----------------------------------------------------------------------------
# Global settings
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# -----------------------------------------------------------------------------
# CMake modules
# -----------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(cmake/CompilerWarnings.cmake)
include(cmake/Platform.cmake)

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
# Qt6 is required for both core library (QtCore, QtNetwork) and GUI
find_package(Qt6 REQUIRED COMPONENTS Core Network Multimedia)

if(SOUND2OSC_BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS
        Gui
        Widgets
        Qml
        Quick
        QuickControls2
    )
endif()

# Enable Qt's automatic handling of moc, uic, and rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# -----------------------------------------------------------------------------
# Third-party libraries
# -----------------------------------------------------------------------------
# FFFT is header-only, just create an interface target
add_library(ffft INTERFACE)
target_include_directories(ffft INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffft/include>
    $<INSTALL_INTERFACE:include>
)

# -----------------------------------------------------------------------------
# Subdirectories
# -----------------------------------------------------------------------------
add_subdirectory(libs)

if(SOUND2OSC_BUILD_GUI OR SOUND2OSC_BUILD_HEADLESS)
    add_subdirectory(apps)
endif()

if(SOUND2OSC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# -----------------------------------------------------------------------------
# Summary
# -----------------------------------------------------------------------------
message(STATUS "")
message(STATUS "sound2osc ${PROJECT_VERSION} configuration:")
message(STATUS "  Build type:      ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard:    ${CMAKE_CXX_STANDARD}")
message(STATUS "  Qt version:      ${Qt6_VERSION}")
message(STATUS "  Build GUI:       ${SOUND2OSC_BUILD_GUI}")
message(STATUS "  Build headless:  ${SOUND2OSC_BUILD_HEADLESS}")
message(STATUS "  Build tests:     ${SOUND2OSC_BUILD_TESTS}")
message(STATUS "")
