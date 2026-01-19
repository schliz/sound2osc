# apps/gui/CMakeLists.txt
# Qt6 GUI application for sound2osc

set(GUI_SOURCES
    main.cpp
    src/controllers/MainController.cpp
    src/controllers/TriggerGuiController.cpp
    src/osc/OSCMapping.cpp
    src/utils/SettingsMigration.cpp
)

set(GUI_HEADERS
    src/controllers/MainController.h
    src/controllers/TriggerGuiController.h
    src/osc/OSCMapping.h
    src/utils/SettingsMigration.h
)

set(GUI_RESOURCES
    resources/qml.qrc
    resources/images.qrc
)

# Create the executable
add_executable(sound2osc
    ${GUI_SOURCES}
    ${GUI_HEADERS}
    ${GUI_RESOURCES}
)

# Include directories
target_include_directories(sound2osc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/osc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
)

# Link against core library and Qt6
target_link_libraries(sound2osc PRIVATE
    sound2osc::core
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Multimedia
    Qt6::Network
)

# Apply compiler warnings
sound2osc_set_warnings(sound2osc)

# Apply platform-specific settings
sound2osc_platform_config(sound2osc)

# Platform-specific configuration
if(WIN32)
    # Windows: create a GUI application (not console)
    set_target_properties(sound2osc PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
elseif(APPLE)
    # macOS: create an app bundle
    set_target_properties(sound2osc PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.sound2osc.app"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
endif()

# Set output directory
set_target_properties(sound2osc PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Installation
install(TARGETS sound2osc
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)
