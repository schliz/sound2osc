# libs/sound2osc-core/CMakeLists.txt
# Core library for sound2osc - contains all business logic without GUI dependencies

set(CORE_SOURCES
    # Audio module
    src/audio/MonoAudioBuffer.cpp

    # DSP module
    src/dsp/FFTAnalyzer.cpp
    src/dsp/ScaledSpectrum.cpp

    # Trigger module
    src/trigger/TriggerFilter.cpp
    src/trigger/TriggerGenerator.cpp
    src/trigger/TriggerOscParameters.cpp

    # BPM module
    src/bpm/BPMDetector.cpp
    src/bpm/BPMTapDetector.cpp
    src/bpm/BPMOscControler.cpp

    # OSC module
    src/osc/OSCParser.cpp
    src/osc/OSCMessage.cpp
    src/osc/OSCNetworkManager.cpp

    # Logging module
    src/logging/Logger.cpp

    # Config module
    src/config/JsonConfigStore.cpp
    src/config/PresetManager.cpp
    src/config/SettingsManager.cpp

    # Core module
    src/core/AppInfo.cpp
    src/core/Sound2OscEngine.cpp
)

set(CORE_HEADERS
    # Audio module
    include/sound2osc/audio/AudioInputInterface.h
    include/sound2osc/audio/MonoAudioBuffer.h

    # DSP module
    include/sound2osc/dsp/BasicFFTInterface.h
    include/sound2osc/dsp/FFTRealWrapper.h
    include/sound2osc/dsp/FFTAnalyzer.h
    include/sound2osc/dsp/ScaledSpectrum.h

    # Trigger module
    include/sound2osc/trigger/TriggerGeneratorInterface.h
    include/sound2osc/trigger/TriggerFilter.h
    include/sound2osc/trigger/TriggerGenerator.h
    include/sound2osc/trigger/TriggerOscParameters.h

    # BPM module
    include/sound2osc/bpm/BPMDetector.h
    include/sound2osc/bpm/BPMTapDetector.h
    include/sound2osc/bpm/BPMOscControler.h

    # OSC module
    include/sound2osc/osc/OSCParser.h
    include/sound2osc/osc/OSCMessage.h
    include/sound2osc/osc/OSCNetworkManager.h

    # Core utilities
    include/sound2osc/core/QCircularBuffer.h
    include/sound2osc/core/utils.h
    include/sound2osc/core/versionInfo.h
    include/sound2osc/core/AppInfo.h
    include/sound2osc/core/Sound2OscEngine.h

    # Logging module
    include/sound2osc/logging/Logger.h

    # Config module
    include/sound2osc/config/ConfigStore.h
    include/sound2osc/config/JsonConfigStore.h
    include/sound2osc/config/PresetManager.h
    include/sound2osc/config/SettingsManager.h
)

if(SOUND2OSC_AUDIO_BACKEND STREQUAL "Miniaudio")
    list(APPEND CORE_SOURCES src/audio/MiniaudioInputWrapper.cpp)
    list(APPEND CORE_HEADERS include/sound2osc/audio/MiniaudioInputWrapper.h)
else()
    list(APPEND CORE_SOURCES src/audio/QAudioInputWrapper.cpp)
    list(APPEND CORE_HEADERS include/sound2osc/audio/QAudioInputWrapper.h)
endif()

add_library(sound2osc-core STATIC
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

add_library(sound2osc::core ALIAS sound2osc-core)

target_include_directories(sound2osc-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(sound2osc-core
    PUBLIC
        Qt6::Core
        Qt6::Network
        ffft
)

if(SOUND2OSC_AUDIO_BACKEND STREQUAL "Miniaudio")
    target_compile_definitions(sound2osc-core PUBLIC SOUND2OSC_USE_MINIAUDIO)
    target_include_directories(sound2osc-core PRIVATE 
        ${CMAKE_SOURCE_DIR}/third_party/miniaudio
    )
    if(UNIX AND NOT APPLE)
        target_link_libraries(sound2osc-core PRIVATE dl pthread)
    endif()
else()
    target_link_libraries(sound2osc-core PUBLIC Qt6::Multimedia)
endif()

# Apply compiler warnings
sound2osc_set_warnings(sound2osc-core)

# Apply platform-specific settings
sound2osc_platform_config(sound2osc-core)

# Set properties
set_target_properties(sound2osc-core PROPERTIES
    OUTPUT_NAME "sound2osc-core"
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)
