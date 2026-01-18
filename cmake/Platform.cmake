# cmake/Platform.cmake
# Platform-specific configuration for sound2osc

# -----------------------------------------------------------------------------
# Platform detection
# -----------------------------------------------------------------------------
if(WIN32)
    set(SOUND2OSC_PLATFORM "Windows")
    set(SOUND2OSC_PLATFORM_WINDOWS TRUE)
elseif(APPLE)
    set(SOUND2OSC_PLATFORM "macOS")
    set(SOUND2OSC_PLATFORM_MACOS TRUE)
elseif(UNIX)
    set(SOUND2OSC_PLATFORM "Linux")
    set(SOUND2OSC_PLATFORM_LINUX TRUE)
else()
    set(SOUND2OSC_PLATFORM "Unknown")
endif()

message(STATUS "Building for platform: ${SOUND2OSC_PLATFORM}")

# -----------------------------------------------------------------------------
# Platform-specific compile definitions
# -----------------------------------------------------------------------------
function(sound2osc_platform_config target)
    if(SOUND2OSC_PLATFORM_WINDOWS)
        target_compile_definitions(${target} PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
            _CRT_SECURE_NO_WARNINGS
        )
    endif()

    if(SOUND2OSC_PLATFORM_MACOS)
        target_compile_definitions(${target} PRIVATE
            GL_SILENCE_DEPRECATION
        )
    endif()
endfunction()

# -----------------------------------------------------------------------------
# RPATH settings for shared libraries
# -----------------------------------------------------------------------------
if(NOT MSVC)
    # Use full RPATH for build tree
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    # Don't use install RPATH when building
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    # Set install RPATH to executable location
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
    # Add build tree locations to RPATH
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()
