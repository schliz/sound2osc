# apps/CMakeLists.txt
# Applications built on top of sound2osc-core

if(SOUND2OSC_BUILD_GUI)
    add_subdirectory(gui)
endif()

# Headless application (CLI/daemon)
if(SOUND2OSC_BUILD_HEADLESS)
    add_subdirectory(headless)
endif()
