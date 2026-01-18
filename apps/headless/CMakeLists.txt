# apps/headless/CMakeLists.txt
# Headless (CLI) application for sound2osc

set(HEADLESS_SOURCES
    main.cpp
)

add_executable(sound2osc-headless
    ${HEADLESS_SOURCES}
)

target_link_libraries(sound2osc-headless
    PRIVATE
        sound2osc::core
        Qt6::Core
        Qt6::Network
        Qt6::Multimedia
)

# Apply compiler warnings
sound2osc_set_warnings(sound2osc-headless)

# Apply platform-specific settings  
sound2osc_platform_config(sound2osc-headless)

# Set output directory to match GUI app
set_target_properties(sound2osc-headless PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
