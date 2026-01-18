# apps/CMakeLists.txt
# Applications built on top of sound2osc-core

add_subdirectory(gui)

# Headless application (CLI/daemon)
if(SOUND2OSC_BUILD_HEADLESS)
    add_subdirectory(headless)
endif()
